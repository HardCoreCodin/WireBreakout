#pragma once

#include "./ball.hpp"
#include "./paddle.hpp"
#include "./level.hpp"

struct BallController {
    static constexpr float DEFAULT_START_POSITION_X = 0;
    static constexpr float DEFAULT_START_POSITION_y = 50;
    static constexpr float DEFAULT_START_VELOCITY_X = 30;
    static constexpr float DEFAULT_START_VELOCITY_y = -35;

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

        ball.velocity.y = speed;
    }

    void update(float delta_time, const vec2 &game_scale, const Paddle &paddle, Brick *bricks, u8 bricks_count) {
        Rect rect;

        if (ball.position.y <= (paddle.rect.top + ball.radius)) {
            // The ball is within range for the paddle:
            // This section is only used for the cases where the ball starts-out already within the paddle bounds.
            // This can happen because the paddle moves irrespective of the ball and does not react to it.
            // So because the paddle movement is updated before the ball, this needs to be resolved explicitly.
            rect = paddle.rect;
            rect.left -= ball.radius;
            rect.right += ball.radius;
            rect.top += ball.radius;
            if (rect[ball.position]) {
                // The ball is in-range for the 'expanded' rectangle that accounts for the ball's radius.
                // Unless it is around the corners, it is either touching the bounds or partially overlapping them.

                // Check if that ball's center is withing the actual bounds of the paddle:
                if (paddle.rect[ball.position]) {
                    // The ball center is inside the paddle.
                    // Push it outward to the paddle's edge and give it some of the paddle's speed:
                    if (paddle.speed_x > 0) {
                        ball.position.x = rect.right;
                        ball.velocity.x += 0.1f * paddle.speed_x;
                    } else {
                        ball.position.x = rect.left;
                        ball.velocity.x -= 0.1f * paddle.speed_x;
                    }
                } else {
                    // The ball center is outside the paddle, but parts of it may still be overlapping.
                    // Do a simplified 'clamping' approach to push it outward appropriately:
                    movement = ball.position - paddle.rect.clamped(ball.position);
                    movement_distance = movement.length();
                    if ((movement_distance < 0 ? -movement_distance : movement_distance) < 0.001f) movement_distance = 0;
                    t_min = ball.radius - movement_distance;
                    if (t_min > 0) {
                        movement /= movement_distance; // normalize
                        ball.position += movement * t_min;
                        if (ball.position.y > paddle.position.y) // Bass is above the paddle - bounce off ot if:
                            ball.velocity = ball.velocity.reflectAround(movement);
                        else {
                            // The ball is to the side of the paddle.
                            // Push it outward to the paddle's edge and give it some of the paddle's speed:
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

        // Iteratively resolve the ball's movement:
        // At each step the closest hit is found and handled while tracking the overall amount the ball has moved
        i32 hit_brick_index;
        t_remaining = 1.0f;
        while (t_remaining) {
            // Each iteration the t_min is reset such that multiple successive intersection-checks can be made
            // so that the closes-hit if found:
            t_min = 1.0f;
            hit_brick_index = -1;

            // Handle level bounds collision in a simple form (collision_position == position_x + t * movement):
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
                        hitBrickRect(bricks[i].rect)) // Test using the Minkowski Sum for Rectangle/Circle
                        hit_brick_index = i; // Track the hit brick index
            } else // Ball is around the paddle:
                hitBrickRect( paddle.rect);  // Test using the Minkowski Sum for Rectangle/Circle

            // If no close-enough collision was detected, keep the currently-set new_position as it is
            if (t_min == 1.0f) t_remaining = 0; else {
                // Move the ball to the closest collision point and reconfigure the variables for the next event:
                old_position += movement * t_min;
                t_remaining -= t_remaining * t_min;
                ball.velocity = ball.velocity.reflectAround(hit_normal);
                movement = ball.velocity * (delta_time * t_remaining);
                movement_distance = movement.length();
                new_position = old_position + movement;
                if (hit_brick_index >= 0) bricks[hit_brick_index].hit(); // If a brick was hit in this iteration, hit it
            }
        }

        ball.position = new_position;
    }

    // Intersection test in local-space of the ball against a perpendicular bound ahead of it
    // Note: This is parameterized to allow it to be reused for both vertical and horizontal bounds
    bool hitPerp(f32 dx, f32 dy, f32 bottom, f32 top, f32 x, f32 &min_t) {
        if (dx == 0.0f) return false; // Bound is parallel to direction
        if (x == 0.0f) { // Bound is at the starting point
            min_t = 0;
            return true;
        }

        f32 t = x / dx; if (t < 0 || t > min_t) return false;
        f32 y = dy * t; if (y > top || y < bottom) return false;
        min_t = t;

        return true;
    }

    f32 hitRect(const Rect &rect) {
        BoxSide side{NoSide};

        // Localize the rectangular bounds relative to the ball:
        f32 top    = rect.top    - old_position.y;
        f32 bottom = rect.bottom - old_position.y;
        f32 left   = rect.left   - old_position.x;
        f32 right  = rect.right  - old_position.x;

        f32 dx = movement.x;
        f32 dy = movement.y;

        f32 min_t = t_min; // Don't overwrite the tracked-t_min just yet as this check may still miss
        // Vertical bounds are checked normatively, horizontal ones are checked by providing axis-mirrored arguments:
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

    // Test using the Minkowski Sum for Rectangle/Circle
    bool hitBrickRect(const Rect &brick_rect) {
        // First check against the expanded rectangular bounds accounting for the ball's radius
        Rect rect{
                brick_rect.left - ball.radius,
                brick_rect.right + ball.radius,
                brick_rect.top + ball.radius,
                brick_rect.bottom - ball.radius
        };

        f32 min_t = hitRect(rect);
        if (min_t) { // If there is a hit, correct for the rounded corners (which may yet still miss) using ray/circle:
            hit_position = old_position + min_t * movement;
            if (hit_position.y > brick_rect.top) {
                if (hit_position.x < brick_rect.left ) return hitCorner({brick_rect.left , brick_rect.top}, min_t);
                if (hit_position.x > brick_rect.right) return hitCorner({brick_rect.right, brick_rect.top}, min_t);
            } else if (hit_position.y < brick_rect.bottom) {
                if (hit_position.x < brick_rect.left ) return hitCorner({brick_rect.left,  brick_rect.bottom}, min_t);
                if (hit_position.x > brick_rect.right) return hitCorner({brick_rect.right, brick_rect.bottom}, min_t);
            }

            // None of the corners early-returned a result, so the hit is outside the corners and is valid:
            t_min = min_t;
            return true;
        } else
            return false;
    }

    // Ray/Circle intersection check against a provided circle-center (positioned at some rectangular corner)
    bool hitCorner(const vec2 &C, f32 min_t) {
        vec2 Rd = new_position - hit_position;
        f32 remaining_distance = Rd.length();
        Rd /= remaining_distance;

        f32 t = Rd.dot(C - hit_position);
        vec2 closest_point = hit_position + t*Rd;
        f32 t2 = ball.radius * ball.radius - (closest_point - C).squaredLength();
        if (t2 <= 0.0) // The closest point to the circle is away from its center by more than its radius - a miss:
            return false;

        f32 distance = t - sqrtf(t2);
        if (distance <= 0) // Hit is behind the ray (technically shouldn't happen in this use case)
            return false;

        // A valid hit was found
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