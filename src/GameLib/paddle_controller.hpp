#pragma once

#include "./ball.hpp"
#include "./paddle.hpp"

struct PaddleController {
    static constexpr float DEFAULT_MAX_VELOCITY = 70;
    static constexpr float DEFAULT_ACCELERATION = 500;
    static constexpr float DEFAULT_START_POSITION = 0;
    static constexpr float DEFAULT_START_VELOCITY = 0;
    static constexpr char DEFAULT_MOVE_LEFT_KEY = 'A';
    static constexpr char DEFAULT_MOVE_RIGHT_KEY ='D';
    static constexpr char DEFAULT_LAUNCH_RIGHT_KEY = 'W';
    static constexpr float DEFAULT_LAUNCH_SPEED = 40;
    static constexpr float DEFAULT_LAUNCH_AREA_RADIUS = 7;

    Paddle &paddle;

    float start_position;
    float start_velocity;
    float max_velocity;
    float acceleration;
    float launch_speed;
    float launch_area_radius;

    char move_left_key;
    char move_right_key;
    char launch_ball_key{};

    bool move_left;
    bool move_right;
    bool launch_ball;

    PaddleController(Paddle &paddle,
                     float start_position = DEFAULT_START_POSITION,
                     float start_velocity = DEFAULT_START_VELOCITY,
                     char move_left_key = DEFAULT_MOVE_LEFT_KEY,
                     char move_right_key = DEFAULT_MOVE_RIGHT_KEY,
                     char launch_ball_key = DEFAULT_LAUNCH_RIGHT_KEY,
                     float launch_speed = DEFAULT_LAUNCH_SPEED,
                     float launch_area_radius = DEFAULT_LAUNCH_AREA_RADIUS,
                     float max_velocity = DEFAULT_MAX_VELOCITY,
                     float acceleration = DEFAULT_ACCELERATION) :
        paddle{paddle},
        start_position{start_position},
        start_velocity{start_velocity},
        launch_speed{launch_speed},
        launch_area_radius{launch_area_radius},
        max_velocity{max_velocity},
        acceleration{acceleration},
        move_left_key{move_left_key},
        move_right_key{move_right_key},
        launch_ball_key{launch_ball_key},
        move_left{false},
        move_right{false},
        launch_ball{false}
    {}

    void reset() {
        paddle.position.x = start_position;
        paddle.speed_x = start_velocity;
        paddle.updateRect();
    }

    void update(float delta_time, float level_bound_x) {
        float target_velocity{0};
        if (move_right) target_velocity += +max_velocity;
        if (move_left) target_velocity += -max_velocity;
        paddle.speed_x = approach(paddle.speed_x, target_velocity, acceleration * delta_time);
        paddle.update(delta_time, level_bound_x);
    }

    void OnKeyChange(u8 key, bool is_pressed) {
        if (key == controls::key_map::left  || key == move_left_key ) move_left  = is_pressed;
        if (key == controls::key_map::right || key == move_right_key) move_right = is_pressed;
        if (key == controls::key_map::up    || key == launch_ball_key) launch_ball = true;
    }
};