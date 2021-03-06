#pragma once

#include "./brick.hpp"

struct Paddle : BrickBase {
    static constexpr ColorID DEFAULT_PADDLE_COLOR = Magenta;
    static constexpr float DEFAULT_PADDLE_SCALE_X = 6;

    Paddle(ColorID color_id = DEFAULT_PADDLE_COLOR,
           float scale_x = DEFAULT_PADDLE_SCALE_X) :
           BrickBase({}, color_id, scale_x)
    {
    }
};