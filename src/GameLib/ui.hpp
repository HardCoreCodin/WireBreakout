#pragma once

#include "../SlimEngine/core/string.h"
#include "../SlimEngine/math/vec2.h"

namespace GameUI {
    static const vec2 title_offset{0.5f, 0.1f};
    static const vec2 start_button_offset{0.5f, 0.4f};
    static const vec2 quit_button_offset{0.5f, 0.7f};

    struct TextBox {
        String text{};
        vec2 offset_percentage{0.0f};
        vec2i position{0}, text_position{0};
        vec3 color{Color(White)};

        TextBox() = default;
        TextBox(char *text, vec2 offset = vec2{0.0f}) : text{text}, offset_percentage{offset} {}
        TextBox(u16 width, u16 height, char *text, vec2 offset = vec2{0.0f}) : TextBox{text, offset} {
            setRelativePosition(width, height);
        }

        void setRelativePosition(u16 width, u16 height) {
            position.x = i32(offset_percentage.x * (f32)width);
            position.y = i32(offset_percentage.y * (f32)height);
            text_position.x = position.x - (FONT_WIDTH - 1) * text.length / 2;
            text_position.y = position.y - (FONT_HEIGHT / 2);
        }
    };

    struct Button : TextBox {
        RectI rect{};
        vec3 background_color{Color(Grey)};
        vec3 border_color{Color(BrightGrey)};
        vec2i padding{5};
        vec2i text_size{FONT_HEIGHT, FONT_WIDTH};
        bool is_pressed{false};

        Button() = default;
        Button(u16 width, u16 height, char *str, vec2 offset = vec2{0.0f}) : TextBox{str, offset} {
            setRelativePosition(width, height);
            text_size.x = text.length * FONT_WIDTH;
        }

        void setRelativePosition(u16 width, u16 height) {
            TextBox::setRelativePosition(width, height);

            vec2i extent{text_size / 2.0f + padding};
            rect.top_left = position - extent;
            rect.bottom_right = position + extent;
        }
    };

    struct Menu {
        TextBox title;
        Button start_button, quit_button;
        vec3 background_color{Color(DarkGrey)};

        Menu(char *title_text, vec2 title_offset,
             char* start_button_text, vec2 start_button_offset,
             char* quit_button_text, vec2 quit_button_offset,
             u16 width,
             u16 height
        ) : title{width, height, title_text, title_offset},
            start_button{width, height, start_button_text, start_button_offset},
            quit_button{width, height, quit_button_text, quit_button_offset}
        {
            OnResize(width, height);
        }

        void OnResize(u16 width, u16 height) {
            start_button.setRelativePosition(width, height);
            quit_button.setRelativePosition(width, height);
        }

        void OnMouseButtonClicked(const vec2i &mouse_position) {
            if (start_button.rect[mouse_position]) start_button.is_pressed = true;
            if (quit_button.rect[ mouse_position]) quit_button.is_pressed = true;
        }
    };

    static Menu start_menu{
              (char*)"Breakout:", title_offset,
        (char*)"Start", start_button_offset,
        (char*)"Quit", quit_button_offset,
        DEFAULT_WIDTH,DEFAULT_HEIGHT,
    };
    static Menu end_menu{
              (char*)"Game Over!", title_offset,
        (char*)"Restart", start_button_offset,
        (char*)"Quit", quit_button_offset,
        DEFAULT_WIDTH,DEFAULT_HEIGHT,
    };
    static Menu level_failed_menu{
              (char*)"You Failed!", title_offset,
        (char*)"Retry", start_button_offset,
        (char*)"Quit", quit_button_offset,
        DEFAULT_WIDTH,DEFAULT_HEIGHT,
    };
    static Menu level_completed_menu{
              (char*)"Well Done!", title_offset,
        (char*)"Continue", start_button_offset,
        (char*)"Quit", quit_button_offset,
        DEFAULT_WIDTH,DEFAULT_HEIGHT,
    };
}