#pragma once

#include "../math/vec2.h"

template<class VectorType, typename scalar_type>
struct RectOf {
    union {
        struct {
            VectorType top_left;
            VectorType bottom_right;
        };
        struct {
            scalar_type left;
            scalar_type top;
            scalar_type right;
            scalar_type bottom;
        };
    };

    RectOf() : RectOf{(scalar_type)0, (scalar_type)0, (scalar_type)0, (scalar_type)0} {}
    RectOf(const RectOf &other) : RectOf{other.top_left, other.bottom_right} {}
    RectOf(const VectorType &top_left,
           const VectorType &bottom_right) :
            top_left{top_left},
            bottom_right{bottom_right} {}
    RectOf(scalar_type left, scalar_type right, scalar_type top, scalar_type bottom) :
            left{left}, top{top}, right{right}, bottom{bottom} {}

    INLINE bool contains(const VectorType &pos) const {
        return pos.x >= left &&
               pos.x <= right &&
               pos.y >= bottom &&
               pos.y <= top;
    }

    INLINE bool bounds(const VectorType &pos) const {
        return pos.x > left &&
               pos.x < right &&
               pos.y > bottom &&
               pos.y < top;
    }

    INLINE bool is_zero() const {
        return left == right && top == bottom;
    }

    INLINE VectorType clamped(const VectorType &vec) const {
        return vec.clamped({left, bottom}, {right, top});
    }

    INLINE bool operator ! () const {
        return is_zero();
    }

    INLINE bool operator [] (const VectorType &pos) const {
        return contains(pos);
    }

    INLINE bool operator () (const VectorType &pos) const {
        return bounds(pos);
    }
};

using Rect = RectOf<vec2, f32>;
using RectI = RectOf<vec2i, i32>;