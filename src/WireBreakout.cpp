#include "./SlimEngine/draw/rectangle.h"
#include "./SlimEngine/draw/curve.h"
#include "./SlimEngine/draw/box.h"
#include "./SlimEngine/draw/hud.h"
#include "./SlimEngine/app.h"

#include "./GameLib/game.hpp"

struct WireBreakout : SlimEngine {
    char *map1{(char*)""
                      "  ~  " "\n"
                      "#= =#" "\n"
                      "=~  =" "\n"
                      "=---="
    };
    char *map2{(char*)""
                      "     " "\n"
                      "#-==*" "\n"
                      "~  -=" "\n"
                      "=~  =" "\n"
                      "-=~ #"
    };
    Brick bricks[64];
    Level level01{map1, bricks};
    Level level02{map2, bricks};
    Game game{&level01, 2};

    // Viewport:
    Camera game_camera{
        {0, 45, -1000},
        {},
        20
    };
    Camera default_editor_camera{
            {0, 15, -100},
            {15 * DEG_TO_RAD, 0, 0}
    };
    Camera editor_camera;
    Viewport viewport{window::canvas, &game_camera};

    // HUD:
    HUDLine Lives{ (char*)"Lives : "};
    HUDLine Bricks{(char*)"Bricks: "};
    HUDSettings hud_settings{
            2,
            1.2f,
            Green
    };
    HUD hud{hud_settings, &Lives};

    // Scene:
    Box box{};
    Curve curve{ CurveType::Coil, 10};

    Transform transform, default_transform{};
    quat ball_orientation{quat::RotationAroundX(90*DEG_TO_RAD)};

    // Drawing:
    f32 opacity = 0.5f;
    u8 line_width = 0;

    WireBreakout() {
        viewport.navigation.settings.max_velocity *= 10;
        viewport.navigation.settings.acceleration *= 10;
    }

    void OnRender() override {
        if (game.current_menu) {
            GameUI::Menu &menu = *game.current_menu;
            GameUI::Button &s = menu.start_button;
            GameUI::Button &q = menu.quit_button;
            GameUI::TextBox &t = menu.title;
            menu.OnResize(viewport.dimensions.width, viewport.dimensions.height);

            window::canvas.fill(game.current_menu->background_color, 1, 0);

            fill(s.rect, viewport, s.background_color);
            fill(q.rect, viewport, q.background_color);
            draw(s.rect, viewport, s.border_color);
            draw(q.rect, viewport, q.border_color);

            drawText(t.text.char_ptr, t.text_position.x, t.text_position.y, viewport, t.color, 1);
            drawText(s.text.char_ptr, s.text_position.x, s.text_position.y, viewport, s.color, 1);
            drawText(q.text.char_ptr, q.text_position.x, q.text_position.y, viewport, q.color, 1);
        } else {
            Level &level = *game.current_level;

            // Draw Bounds:
            transform = default_transform;
            transform.position.x = level.scale.x + 1;
            transform.position.y = transform.scale.y = level.scale.y;
            draw(box, transform, viewport, level.bounds_color, opacity, line_width);
            transform.position.x = -transform.position.x;
            draw(box, transform, viewport, level.bounds_color, opacity, line_width);
            transform = default_transform;
            transform.position.y = level.scale.y * 2 + 1;
            transform.scale.x = level.scale.x + 2;
            draw(box, transform, viewport, level.bounds_color, opacity, line_width);

            // Draw Level:
            transform = default_transform;
            transform.scale.x = level.scale.x + 2;
            for (u32 i = 0; i < level.bricks_count; i++) {
                Brick &brick = level.bricks[i];
                if (brick.is_broken()) continue;
                transform.position.x = brick.position.x;
                transform.position.y = brick.position.y;
                transform.scale.x = brick.scale_x;
                draw(box, transform, viewport, Color(brick.color_id), opacity, line_width);
            }

            // Draw Paddle:
            transform = default_transform;
            transform.scale.x = game.paddle.scale_x;
            transform.position.x = game.paddle.position.x;
            draw(box, transform, viewport, Color(game.paddle.color_id), opacity, line_width);

            // Draw Ball:
            transform = default_transform;
            transform.scale = game.ball.radius;
            transform.rotation = ball_orientation;
            transform.position.x = game.ball.position.x;
            transform.position.y = game.ball.position.y;
            draw(curve, transform, viewport, Color(game.ball.color_id), opacity, line_width);

            // Draw HUD:
            Lives.value = (i32)game.lives;
            Bricks.value = (i32)game.current_level->breakable_bricks_count;
            draw(hud, viewport);

            if (game.is_paused) {
                GameUI::TextBox &t = GameUI::game_paused_text;
                t.setRelativePosition(viewport.dimensions.width, viewport.dimensions.height);
                drawText(t.text.char_ptr, t.text_position.x, t.text_position.y, viewport, t.color, 1);
            }

        }
    }
    void OnWindowResize(u16 width, u16 height) override {
        viewport.updateDimensions(width, height);
    }

    void OnUpdate(f32 delta_time) override {
        if (game.is_paused)
            viewport.updateNavigation(delta_time);
        else
            game.OnUpdate(delta_time);
    }
    void OnMouseButtonDown(mouse::Button &mouse_button) override {
        mouse::pos_raw_diff_x = mouse::pos_raw_diff_y = 0;
        if (&mouse_button == &mouse::left_button) {
            vec2i mouse_position{mouse::pos_x, mouse::pos_y};
            if (!game.OnMouseButtonClicked(mouse_position))
                is_running = false;
        }
    }
    void OnMouseButtonDoubleClicked(mouse::Button &mouse_button) override {
        if (game.is_paused && &mouse_button == &mouse::left_button) {
            mouse::is_captured = !mouse::is_captured;
            os::setCursorVisibility(!mouse::is_captured);
            os::setWindowCapture(    mouse::is_captured);
            OnMouseButtonDown(mouse_button);
        }
    }
    void OnKeyChanged(u8 key, bool is_pressed) override {
        if (controls::is_pressed::escape) {
            is_running = false;
            return;
        }
        if (key == controls::key_map::space && is_pressed && !mouse::is_captured) {
            game.is_paused = !game.is_paused;
            if (game.is_paused) {
                editor_camera = default_editor_camera;
                viewport.setCamera(editor_camera);
            } else {
                viewport.setCamera(game_camera);
            }
        }
        if (game.is_paused) {
            NavigationMove &move = viewport.navigation.move;
            NavigationTurn &turn = viewport.navigation.turn;
            if (key == 'Q') turn.left     = is_pressed;
            if (key == 'E') turn.right    = is_pressed;
            if (key == 'R') move.up       = is_pressed;
            if (key == 'F') move.down     = is_pressed;
            if (key == 'W') move.forward  = is_pressed;
            if (key == 'S') move.backward = is_pressed;
            if (key == 'A') move.left     = is_pressed;
            if (key == 'D') move.right    = is_pressed;
        } else
            game.OnKeyChanged(key, is_pressed);
    }
};

SlimEngine* createEngine() {
    return (SlimEngine*)new WireBreakout();
}