#pragma once

#include "../SlimEngine/math/vec2.h"

struct Ball {
    static constexpr float DEFAULT_RADIUS = 1;
    static constexpr ColorID DEFAULT_COLOR = Cyan;

    vec2 position;
    vec2 velocity;
    float radius;
    ColorID color_id;

    Ball(float radius = DEFAULT_RADIUS, ColorID color_id = DEFAULT_COLOR) :
        position{0},
        velocity{0},
        radius{radius},
        color_id(color_id)
    {}
};