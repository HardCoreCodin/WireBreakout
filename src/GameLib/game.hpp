#pragma once

#include "./ball_controller.hpp"
#include "./paddle_controller.hpp"
#include "./level.hpp"
#include "./ui.hpp"

struct Game {
    Level *levels;
    Level *current_level;
    u8 current_level_index;
    u8 levels_count;

    GameUI::Menu *current_menu = &GameUI::start_menu;

    u8 starting_lives = 5;
    u8 lives = starting_lives;
    bool is_paused = false;

    Ball ball;
    Paddle paddle;

    BallController ball_controller{ball};
    PaddleController paddle_controller{paddle};

    Game(Level *levels, u8 levels_count) :
        levels{levels},
        current_level{levels},
        current_level_index{0},
        levels_count{levels_count}
    {}

    void die() {
        lives--;
        if (lives)
            ball_controller.reset();
        else
            failLevel();
    }

    void endGame() { current_menu = &GameUI::end_menu; }
    void startGame() {
        current_level_index = 0;
        current_level = levels;
        resetLives();
        startLevel();
    }
    void startLevel() {
        current_menu = nullptr;
        current_level->reset();
        paddle_controller.reset();
        ball_controller.reset();
    }

    void failLevel() {
        current_menu = &GameUI::level_failed_menu;
    }

    void completeLevel() {
        current_level_index++;
        if (current_level_index == levels_count)
            endGame();
        else {
            current_level++;
            current_menu = &GameUI::level_completed_menu;
        }
    }

    void OnUpdate(f32 delta_time) {
        if (current_menu) {
            if (current_menu->start_button.is_pressed) {
                current_menu->start_button.is_pressed = false;
                if (     current_menu == &GameUI::end_menu) startGame();
                else if (current_menu == &GameUI::level_failed_menu) resetLives();
                startLevel();
            }
        } else
            updatePlay(delta_time);
    }

    void OnKeyChanged(u8 key, bool is_pressed) {
        paddle_controller.OnKeyChange(key, is_pressed);
    }

    bool OnMouseButtonClicked(const vec2i &mouse_position) {
        if (current_menu) {
            current_menu->OnMouseButtonClicked(mouse_position);
            if (current_menu->quit_button.is_pressed)
                return false;
        }
        return true;
    }

protected:

    void resetLives() {
        lives = starting_lives;
    }

    void updatePlay(f32 delta_time) {
//        Paddle old_paddle = paddle;
        paddle_controller.update(delta_time, current_level->scale.x);
//        if (ball.position.y - ball.radius < paddle.position.y) {
//            if ()
//        }

        if (paddle_controller.launch_ball)
            ball_controller.launch(paddle_controller.launch_speed,
                                   paddle_controller.launch_area_radius,
                                   paddle.position.x);

        current_level->updateMovingBricks(delta_time);

        ball_controller.update(delta_time,
                               current_level->scale, paddle,
                               current_level->bricks,
                               current_level->bricks_count);

        current_level->updateBricks();
        if (!current_level->breakable_bricks_count)
            completeLevel();

        paddle_controller.launch_ball = false;
        if (ball.position.y < -5) die();
    }
};