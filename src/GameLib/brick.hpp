#pragma once

#include "../SlimEngine/core/rectangle.h"

struct BrickBase {
    static constexpr ColorID DEFAULT_COLOR = BrightGreen;
    static constexpr float DEFAULT_SCALE_X = 4;

    Rect rect;
    vec2 position;
    ColorID color_id;

    float scale_x;
    float speed_x;

    BrickBase(vec2 position = {},
          ColorID color_id = DEFAULT_COLOR,
          float scale_x = DEFAULT_SCALE_X,
          float speed_x = 0) :
          rect{{0.0f, 0.0f}, {0.0f, 0.0f}},
          position{position},
          color_id{color_id},
          scale_x{scale_x},
          speed_x{speed_x}
    {
        updateRect();
    }

    void updateRect() {
        rect.left = position.x - scale_x;
        rect.right = position.x + scale_x;
        rect.top = position.y + 1;
        rect.bottom = position.y - 1;
    }

    void update(float delta_time, float level_bound_x, bool flip_direction = false) {
        level_bound_x -= scale_x;
        float movement = speed_x * delta_time;
        if (movement > 0) { // Sliding right:
            f32 distance = level_bound_x - position.x;
            if (distance < 0 || // Behind
                distance > movement) // Too far in front
                position.x += movement;
            else {
                if (flip_direction) {
                    speed_x = -speed_x;
                    position.x = level_bound_x + distance;
                } else {
                    speed_x = 0;
                    position.x = level_bound_x;
                }
            }
        } else if (movement < 0) { // Sliding left:
            f32 distance = position.x + level_bound_x;
            if (distance < 0 || // Behind
                distance > -movement) // Too far in front
                position.x += movement;
            else {
                if (flip_direction) {
                    speed_x = -speed_x;
                    position.x = -level_bound_x + distance;
                } else {
                    speed_x = 0;
                    position.x = -level_bound_x;
                }
            }
        }

        updateRect();
    }
};


struct Brick : BrickBase {
    static constexpr ColorID DEFAULT_MOVING_COLOR = Yellow;
    static constexpr ColorID DEFAULT_STRONG_COLOR = Green;
    static constexpr ColorID DEFAULT_UNBREAKABLE_COLOR = BrightGrey;
    static constexpr ColorID DEFAULT_HIT_COLOR = Red;
    static constexpr char DEFAULT_HITS_TO_BREAK = 2;
    static constexpr float DEFAULT_SPEED_X = 10.0f;

    static Brick Unbreakable(vec2 position) {
        return {
                position,
                -1,
                0,
                DEFAULT_UNBREAKABLE_COLOR
        };
    }

    static Brick Strong(vec2 position) {
        return {
                position,
                DEFAULT_HITS_TO_BREAK,
                0,
                DEFAULT_STRONG_COLOR
        };
    }
    static Brick Moving(vec2 position) {
        return {
                position,
                1,
                DEFAULT_SPEED_X,
                DEFAULT_MOVING_COLOR
        };
    }

    ColorID hit_color_id;
    char hits_to_break;

    Brick(vec2 position = {},
          char hits_to_break = 1,
          float speed_x = 0,
          ColorID color_id = DEFAULT_COLOR,
          ColorID hit_color_id = DEFAULT_HIT_COLOR,
          float scale_x = DEFAULT_SCALE_X) : BrickBase{position, color_id, scale_x, speed_x},
            hit_color_id{hit_color_id},
            hits_to_break{hits_to_break}
    {}

    void hit() {
        if (hits_to_break > 0) {
            hits_to_break--;
            color_id = hit_color_id;
        }
    }
    inline bool is_broken() { return hits_to_break == 0; }
    inline bool is_breakable() { return hits_to_break != -1; }
    inline bool is_movable() { return speed_x != 0; }
};
