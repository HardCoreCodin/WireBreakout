#pragma once

#include "../SlimEngine/core/ray_rectangle.h"

#include "./ball.hpp"
#include "./paddle.hpp"
#include "./level.hpp"

struct BallController {
    static constexpr float DEFAULT_START_POSITION_X = 0;
    static constexpr float DEFAULT_START_POSITION_y = 30;
    static constexpr float DEFAULT_START_VELOCITY_X = 30;
    static constexpr float DEFAULT_START_VELOCITY_y = 30;

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
        old_position = ball.position;
        movement = ball.velocity * delta_time;
        new_position = old_position + movement;

        float x_bound = game_scale.x - ball.radius;
        float y_bound = game_scale.y * 2 - ball.radius;

        Rect rect;
        RayHitOf<vec2> closest_hit;
        Brick *hit_brick;
        t_remaining = 1.0f;
        while (t_remaining) {
            ray.origin = old_position;
            ray.direction = new_position - old_position;
            closest_hit.distance = INFINITY;
            hit_brick = nullptr;

            // Handle bounds collision (collision_position == position_x + t * movement):
            if (new_position.x >=  x_bound) {
                ray.hit.distance = (x_bound - old_position.x) / movement.x;
                if (ray.hit.distance < 1 &&  // No too far
                    ray.hit.distance < closest_hit.distance) // Closer than before)
                {
                    closest_hit.distance = ray.hit.distance;
                    closest_hit.position = ray[ray.hit.distance];
                    closest_hit.normal = {-1.0f, 0};
                }
            }
            if (new_position.x <= -x_bound) {
                ray.hit.distance = (-x_bound - old_position.x) / movement.x;
                if (ray.hit.distance < 1 &&  // No too far
                    ray.hit.distance < closest_hit.distance) // Closer than before)
                {
                    closest_hit.distance = ray.hit.distance;
                    closest_hit.position = ray[ray.hit.distance];
                    closest_hit.normal = {1.0f, 0};
                }
            }

            if (new_position.y >=  y_bound) {
                ray.hit.distance = (y_bound - old_position.y) / movement.y;
                if (ray.hit.distance < 1 &&  // No too far
                    ray.hit.distance < closest_hit.distance) // Closer than before)
                {
                    closest_hit.distance = ray.hit.distance;
                    closest_hit.position = ray[ray.hit.distance];
                    closest_hit.normal = {0.0f, -1.0f};
                }
            }

            if (ball.velocity.y < 0 && new_position.y <= (1 + ball.radius)) { // Ball is approaching the paddle:
                rect.top = paddle.rect.top + ball.radius;
                rect.bottom = paddle.rect.bottom - ball.radius;
                rect.right = paddle.rect.right + ball.radius;
                rect.left = paddle.rect.left - ball.radius;

                if (rayHitRect(ray, rect) && // Hit was found
                    ray.hit.distance < 1 &&  // No too far
                    ray.hit.distance < closest_hit.distance) // Closer than before
                    closest_hit = ray.hit;
            } else if (ball.velocity.y > 0 && new_position.y > game_scale.y) { // Ball is approaching the bricks:
                Brick *brick = bricks;
                for (u32 i = 0; i < bricks_count; i++, brick++) {
                    rect.top = brick->rect.top + ball.radius;
                    rect.bottom = brick->rect.bottom - ball.radius;
                    rect.right = brick->rect.right + ball.radius;
                    rect.left = brick->rect.left - ball.radius;

                    if (rayHitRect(ray, rect) && // Hit was found
                        ray.hit.distance < 1 &&  // No too far
                        ray.hit.distance < closest_hit.distance) // Closer than before
                    {
                        closest_hit = ray.hit;
                        hit_brick = brick;
                    }
                }
            }

            if (closest_hit.distance < INFINITY) {
                t_remaining -= t_remaining * closest_hit.distance;
                ball.velocity = ball.velocity.reflectAround(closest_hit.normal);
                movement = ball.velocity * (delta_time * t_remaining);
                old_position = closest_hit.position;
                new_position = old_position + movement;
                if (hit_brick) hit_brick->hit();
            } else t_remaining = 0;
        }
//
//        // Handle bounds collision (collision_position == position_x + t * movement):
//        if (new_position.x >=  x_bound) _bounce(true,  ( x_bound - old_position.x) / movement.x);
//        if (new_position.x <= -x_bound) _bounce(true,  (-x_bound - old_position.x) / movement.x);
//        if (new_position.y >=  y_bound) _bounce(false, ( y_bound - old_position.y) / movement.y);
//        if (new_position.y <= (1 + ball.radius) && ball.velocity.y < 0) { // Ball is approaching the paddle:
//            if (new_position.y < -1) {
//                // The ball is moving too fast and would tunnel through the paddle using simple collision detection.
//                // Cast a ray instead to see where it might hit the paddle on the way:
//                ray.origin = old_position;
//                ray.direction = new_position - old_position;
//                if (rayHitRect(ray, paddle.rect)) {
//                    if (ray.hit.position.y < 1) { // Hit on the side, bounce horizontally:
//                        if (ray.direction.x > 0) { // Moving right:
////                            ray.hit.position_x.x - ball.radius
//                        } else { // Moving left"
//
//                        }
//                    } else { // Hit at the top, bounce vertically:
//
//                    }
//                    float t = (ray.hit.position.y - old_position.y) / -ray.direction.y;
//
//                }
//            } else {
//                // Compute a position_x on the rectangle that is closes to the ball's center:
//                ray.hit.position = paddle.rect.clamped(new_position);
//                movement = new_position - ray.hit.position;
//                float t = movement.length();
//                if ((t < 0 ? -t : t) < 0.001f) t = 0;
//                float offset = ball.radius - t;
//                if (offset > 0) {
//                    movement /= t; // normalize
//                    new_position += movement * offset;
//                    ball.velocity = ball.velocity.reflectAround(movement);
//                }
//            }
//            if (resolveCollisionWith(paddle.rect))
//                ball.velocity.x += paddle.speed_x * 0.1f;
//        } else if (new_position.y > game_scale.y && ball.velocity.y > 0) { // Ball is approaching the bricks:
//            for (u32 i = 0; i < bricks_count; i++) {
//                if (resolveCollisionWith(bricks[i].rect)) bricks[i].hit();
//            }
//        }
        ball.position = new_position;
    }

//    bool resolveCollisionWith(const Rect &rect) {
//        // Compute a position_x on the rectangle that is closes to the ball's center:
//        movement = new_position - rect.clamped(new_position);
//        float distance = movement.length();
//        if ((distance < 0 ? -distance : distance) < 0.001f) distance = 0;
//        float t = ball.radius - distance;
//        if (t > 0) {
//            movement /= distance; // normalize
//            new_position += movement * t;
//            ball.velocity = ball.velocity.reflectAround(movement);
//            return true;
//        } else
//            return false;
//    }

private:
    RayOf<vec2> ray;
    float t_remaining;
    vec2 movement, old_position, new_position;

//    void _bounce(bool horizontally, float t) {
//        t_remaining -= t_remaining * t;
//        old_position += t * movement;
//        movement *= 1.0f - t;
//        if (horizontally) {
//            movement.x = -movement.x;
//            ball.velocity.x = -ball.velocity.x;
//        } else {
//            movement.y = -movement.y;
//            ball.velocity.y = -ball.velocity.y;
//        }
//        new_position = old_position + movement;
//    }
};