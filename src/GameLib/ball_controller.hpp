#pragma once

//#include "../SlimEngine/core/ray_rectangle.h"

#include "./ball.hpp"
#include "./paddle.hpp"
#include "./level.hpp"

struct BallController {
    static constexpr float DEFAULT_START_POSITION_X = 0;
    static constexpr float DEFAULT_START_POSITION_y = 50;
    static constexpr float DEFAULT_START_VELOCITY_X = 20;
    static constexpr float DEFAULT_START_VELOCITY_y = -25;

    Ball &ball;

    vec2 start_position;
    vec2 start_velocity;

    BallController(Ball &ball,
                   vec2 start_position = {DEFAULT_START_POSITION_X, DEFAULT_START_POSITION_y},
                   vec2 start_velocity = {DEFAULT_START_VELOCITY_X, DEFAULT_START_VELOCITY_y}) :
        ball{ball},
        start_position{start_position},
        start_velocity{start_velocity}
    {}

    void reset() {
        ball.position = start_position;
        ball.velocity = start_velocity;
    }

    void launch(float speed, float area_radius, float paddle_position_x) {
        float ball_distance = vec2{ball.position.x - paddle_position_x, ball.position.y}.length();
        if (ball_distance > area_radius)
            return;

        // If the ball is coming it hot, and the launch was invoked on the way in,
        // then accumulating the velocities may end-up counteracting them and stopping the ball (undesired).
        // Conversely if it's coming in hot and leaving hot and the launch was invoked on the way out,
        // then accumulating the velocities may end-up with a doubling effect and get overblown (also undesired).
        // So, only accumulate when it makes sense, otherwise just set the launch speed by itself:
        if (speed > ball.velocity.y)
            ball.velocity.y = speed;
        else
            ball.velocity.y += speed;
    }

    void update(float delta_time, const vec2 &game_scale, const Paddle &paddle, Brick *bricks, u8 bricks_count) {
        Rect rect;
        if (ball.position.y <= (paddle.rect.top + ball.radius)) {
            rect = paddle.rect;
            rect.left -= ball.radius;
            rect.right += ball.radius;
            rect.top += ball.radius;
            if (rect[ball.position]) {
                if (paddle.rect[ball.position]) {
                    if (paddle.speed_x > 0) {
                        ball.position.x = rect.right;
                        ball.velocity.x += 0.1f * paddle.speed_x;
                    } else {
                        ball.position.x = rect.left;
                        ball.velocity.x -= 0.1f * paddle.speed_x;
                    }
                } else {
                    movement = ball.position - paddle.rect.clamped(ball.position);
                    movement_distance = movement.length();
                    if ((movement_distance < 0 ? -movement_distance : movement_distance) < 0.001f) movement_distance = 0;
                    t_min = ball.radius - movement_distance;
                    if (t_min > 0) {
                        movement /= movement_distance; // normalize
                        ball.position += movement * t_min;
                        if (ball.position.y > paddle.position.y)
                            ball.velocity = ball.velocity.reflectAround(movement);
                        else {
                            if (paddle.speed_x > 0) {
                                ball.position.x = rect.right;
                                ball.velocity.x += 0.1f * paddle.speed_x;
                            } else {
                                ball.position.x = rect.left;
                                ball.velocity.x -= 0.1f * paddle.speed_x;
                            }
                        }
                    }
                }
            }
        }

        old_position = ball.position;
        movement = ball.velocity * delta_time;
        movement_distance = movement.length();
        new_position = old_position + movement;

        float x_bound = game_scale.x - ball.radius;
        float y_bound = game_scale.y * 2 - ball.radius;
//        Rect bounds{
//            ball.radius - game_scale.x,
//            game_scale.x - ball.radius,
//            game_scale.y * 2.0f - ball.radius,
//            -20.0f
//        };
//        Rect rect;
        i32 hit_brick_index;
        t_remaining = 1.0f;
        while (t_remaining) {
            t_min = 1.0f;
            hit_brick_index = -1;

            // Handle bounds collision (collision_position == position_x + t * movement):
            if (new_position.x >=  x_bound) {
                f32 t = (x_bound - old_position.x) / movement.x;
                if (t < t_min) {
                    t_min = t;
                    hit_normal = {-1.0f, 0};
                }
            }
            if (new_position.x <= -x_bound) {
                f32 t = (-x_bound - old_position.x) / movement.x;
                if (t < t_min) {
                    t_min = t;
                    hit_normal = {1.0f, 0};
                }
            }

            if (new_position.y >=  y_bound) {
                f32 t = (y_bound - old_position.y) / movement.y;
                if (t < t_min) {
                    t_min = t;
                    hit_normal = {0.0f, -1.0f};
                }
            }

            if (new_position.y > game_scale.y) { // Ball is around the bricks:
                for (i32 i = 0; i < bricks_count; i++)
                    if (!bricks[i].is_broken() &&
                        hitBrickRect(bricks[i].rect)) hit_brick_index = i;
            } else
                hitBrickRect( paddle.rect);

//            if (ball.velocity.y < 0 && new_position.y <= (1 + ball.radius)) // Ball is approaching the paddle:
//                hitBrickRect( paddle.rect);

            if (t_min == 1.0f) t_remaining = 0; else {
                old_position += movement * t_min;
                t_remaining -= t_remaining * t_min;
                ball.velocity = ball.velocity.reflectAround(hit_normal);
                movement = ball.velocity * (delta_time * t_remaining);
                movement_distance = movement.length();
                new_position = old_position + movement;
                if (hit_brick_index >= 0) bricks[hit_brick_index].hit();
            }
        }

        ball.position = new_position;


    }

    bool hitPerp(f32 dx, f32 dy, f32 bottom, f32 top, f32 x, f32 &min_t) {
        if (dx == 0.0f) return false; // Bound is parallel to direction
        if (x == 0.0f) { // Bound is at the starting point
            min_t = 0;
            return true;
        }
//                signbit(dx) != signbit(x)  // Bound is behind
//                (signbit(dx) ? dx < x : x < dx)
//                )
//            return false;

        f32 t = x / dx; if (t < 0 || t > min_t) return false;
        f32 y = dy * t; if (y > top || y < bottom) return false;
        min_t = t;

        return true;
    }

    f32 hitRect(const Rect &rect) {
        BoxSide side{NoSide};

        f32 top    = rect.top    - old_position.y;
        f32 bottom = rect.bottom - old_position.y;
        f32 left   = rect.left   - old_position.x;
        f32 right  = rect.right  - old_position.x;

        f32 dx = movement.x;
        f32 dy = movement.y;

        f32 min_t = t_min;
        if (hitPerp(dx, dy, bottom, top, left, min_t)) side = BoxSide::Left;
        if (hitPerp(dx, dy, bottom, top, right, min_t)) side = BoxSide::Right;
        if (hitPerp(dy, dx, left, right, top, min_t)) side = BoxSide::Top;
        if (hitPerp(dy, dx, left, right, bottom, min_t)) side = BoxSide::Bottom;

        switch (side) {
            case BoxSide::Right: hit_normal = {1.0f, 0.0f}; return min_t;
            case BoxSide::Left: hit_normal = {-1.0f, 0.0f}; return min_t;
            case BoxSide::Top: hit_normal = {0.0f, 1.0f}; return min_t;
            case BoxSide::Bottom: hit_normal = {0.0f, -1.0f}; return min_t;
            default: return 0;
        }
    }

    bool hitBrickRect(const Rect &brick_rect) {
        Rect rect{
                brick_rect.left - ball.radius,
                brick_rect.right + ball.radius,
                brick_rect.top + ball.radius,
                brick_rect.bottom - ball.radius
        };

        f32 min_t = hitRect(rect);
        if (min_t) {
            hit_position = old_position + min_t * movement;
            if (hit_position.y > brick_rect.top) {
                if (hit_position.x < brick_rect.left ) return hitCorner({brick_rect.left , brick_rect.top}, min_t);
                if (hit_position.x > brick_rect.right) return hitCorner({brick_rect.right, brick_rect.top}, min_t);
            } else if (hit_position.y < brick_rect.bottom) {
                if (hit_position.x < brick_rect.left ) return hitCorner({brick_rect.left,  brick_rect.bottom}, min_t);
                if (hit_position.x > brick_rect.right) return hitCorner({brick_rect.right, brick_rect.bottom}, min_t);
            }

            t_min = min_t;
            return true;
        } else
            return false;
    }

    bool hitCorner(const vec2 &C, f32 min_t) {
        vec2 Rd = new_position - hit_position;
        f32 remaining_distance = Rd.length();
        Rd /= remaining_distance;

        f32 t = Rd.dot(C - hit_position);
        vec2 closest_point = hit_position + t*Rd;
        f32 t2 = ball.radius * ball.radius - (closest_point - C).squaredLength();
        if (t2 <= 0.0) return false;
        f32 distance = t - sqrtf(t2);
        if (distance <= 0) return false;
        if (remaining_distance < distance) distance = remaining_distance;
        t_min = min_t + distance / movement_distance;
        hit_position += distance * Rd;
        hit_normal = (hit_position - C).normalized();
        return true;
    }

private:
    float t_remaining, t_min, movement_distance;
    vec2 movement, old_position, new_position, hit_position, hit_normal;
};