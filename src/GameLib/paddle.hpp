#pragma once

#include "./brick.hpp"

struct Paddle : BrickBase {
    static constexpr ColorID DEFAULT_PADDLE_COLOR = BrightBlue;
    static constexpr ColorID DEFAULT_PADDLE_CAN_LAUNCH_COLOR = Magenta;
    static constexpr float DEFAULT_PADDLE_SCALE_X = 6;

    ColorID default_color_id;
    ColorID can_launch_color_id;

    Paddle(ColorID color_id = DEFAULT_PADDLE_COLOR,
           ColorID launch_color_id = DEFAULT_PADDLE_CAN_LAUNCH_COLOR,
           float scale_x = DEFAULT_PADDLE_SCALE_X) :
           BrickBase({}, color_id, scale_x)
    {
        default_color_id = color_id;
        can_launch_color_id = launch_color_id;
    }
};