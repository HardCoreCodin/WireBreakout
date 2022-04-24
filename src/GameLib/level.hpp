#pragma once

#include "./brick.hpp"

struct Level {
    static constexpr f32 DEFAULT_SCALE_X = 30;
    static constexpr f32 DEFAULT_SCALE_Y = 40;
    static constexpr ColorID DEFAULT_BOUNDS_COLOR = White;

    static constexpr float DEFAULT_BRICKS_PADDING = 2;
    static constexpr float DEFAULT_TOP_PADDING = 10;
    static constexpr float DEFAULT_SIDE_PADDING = 6;

    static constexpr char NO_BRICK_CHAR = ' ';
    static constexpr char BRICK_CHAR = '-';
    static constexpr char STRONG_BRICK_CHAR = '=';
    static constexpr char MOVING_BRICK_CHAR = '~';
    static constexpr char UNBREAKABLE_BRICK_CHAR = '#';

    char *map;
    Brick *bricks;

    vec2 scale;
    vec3 bounds_color;
    float top_padding;
    float side_padding;
    float bricks_padding;
    u8 bricks_count;
    u8 breakable_bricks_count;

    Level(char *map, Brick *bricks,
          vec2 scale = {DEFAULT_SCALE_X, DEFAULT_SCALE_Y},
          ColorID bounds_color_id = DEFAULT_BOUNDS_COLOR,
          float bricks_padding = DEFAULT_BRICKS_PADDING,
          float top_padding = DEFAULT_TOP_PADDING,
          float side_padding = DEFAULT_SIDE_PADDING) :
        map{map},
        bricks{bricks},
        scale{scale},
        bounds_color{Color(bounds_color_id)},
        top_padding{top_padding},
        side_padding{side_padding},
        bricks_padding{bricks_padding}
    {
        reset();
    }

    void reset() {
        char *brick_char = map;
        float left = side_padding + Brick::DEFAULT_SCALE_X - scale.x;
        float top = scale.y * 2 - top_padding - 1;
        vec2 brick_position{left, top};

        breakable_bricks_count = 0;
        bricks_count = 0;
        Brick *brick = bricks;
        while (*brick_char) {
            switch (*brick_char) {
                case UNBREAKABLE_BRICK_CHAR:
                    *brick = Brick::Unbreakable(brick_position);
                    brick++;
                    bricks_count++;
                    break;
                case MOVING_BRICK_CHAR:
                    *brick = Brick::Moving(brick_position);
                    brick++;
                    bricks_count++;
                    breakable_bricks_count++;
                    break;
                case STRONG_BRICK_CHAR:
                    *brick = Brick::Strong(brick_position);
                    brick++;
                    bricks_count++;
                    breakable_bricks_count++;
                    break;
                case BRICK_CHAR:
                    *brick = Brick{brick_position};
                    brick++;
                    bricks_count++;
                    breakable_bricks_count++;
                    break;
            }
            if (*brick_char == '\n') {
                brick_position.x = left;
                brick_position.y += 2 + bricks_padding;
            } else brick_position.x += Brick::DEFAULT_SCALE_X * 2 + bricks_padding;

            brick_char++;
        }
    }

    void updateBricks() {
        for (u32 i = 0; i < bricks_count; i++) {
            Brick &brick = bricks[i];
            if (brick.is_broken()) { // Remove broken brick:
                breakable_bricks_count--;
                brick = bricks[--bricks_count];
            }
        }
    }

    void updateMovingBricks(float delta_time) {
        for (u32 i = 0; i < bricks_count; i++) {
            Brick &brick = bricks[i];
            if (brick.is_movable()) {
                brick.update(delta_time, scale.x, true);

                for (u32 j = 0; j < bricks_count; j++) {
                    if (j == i) continue;
                    Brick &other_brick = bricks[j];
                    if (brick.position.y != other_brick.position.y) continue;

                    float gap = 0;
                    if (
                            brick.speed_x > 0 && // Sliding right
                            brick.position.x < other_brick.position.x // Other brick is on the right
                    ) gap = (
                                (other_brick.position.x - other_brick.scale_x) // Left side of the other brick, right side of the gap
                                -
                                (brick.position.x + brick.scale_x) // Right side of the brick, left side of the gap
                            );
                    else if (
                            brick.speed_x < 0 && // Sliding left
                            other_brick.position.x < brick.position.x // Other brick is on the left
                    ) gap = (
                                (brick.position.x - brick.scale_x) // Left side of the brick, right side of the gap
                                -
                                (other_brick.position.x + other_brick.scale_x) // Right side of the other brick, left side of the gap
                    );
                    if (gap < 0) {
                        brick.position.x += gap * (brick.speed_x > 0 ? 2.0f : -2.0f);
                        brick.speed_x = -brick.speed_x;
                    }
                }
                brick.updateRect();
            }
        }
    }
};