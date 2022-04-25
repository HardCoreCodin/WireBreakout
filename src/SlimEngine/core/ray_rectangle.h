#pragma once

#include "./rectangle.h"

bool hitPerp(f32 Dx, f32 Dy, f32 MinY, f32 MaxY, f32 AtX, f32 &min_t, f32 &Px, f32 &Py) {
    if (
            Dx == 0.0f || // Bound is parallel to direction
            AtX == 0.0f || // Bound is at the starting point
            signbit(Dx) != signbit(AtX) // Bound is behind
        )
        return false;

    f32 t = Dx / AtX; if (t >= min_t) return false;
    f32 y = Dy * t; if (y > MaxY || y < MinY) return false;

    min_t = t;
    Py = y;
    Px = Dx * t;

    return true;
}

bool hitRect(Rect rect, RayOf<vec2> &ray) {
    rect.top_left -= ray.origin;
    rect.bottom_right -= ray.origin;

    BoxSide side{NoSide};

    if (hitPerp(ray.direction.x,
                ray.direction.y,
                rect.bottom,
                rect.top,
                rect.left,
                ray.hit.distance,
                ray.hit.position.x,
                ray.hit.position.y))
        side = BoxSide::Left;

    if (hitPerp(ray.direction.x,
                ray.direction.y,
                rect.bottom,
                rect.top,
                rect.right,
                ray.hit.distance,
                ray.hit.position.x,
                ray.hit.position.y))
        side = BoxSide::Right;

    if (hitPerp(ray.direction.y,
                ray.direction.x,
                rect.left,
                rect.right,
                rect.top,
                ray.hit.distance,
                ray.hit.position.y,
                ray.hit.position.x))
        side = BoxSide::Top;

    if (hitPerp(ray.direction.y,
                ray.direction.x,
                rect.left,
                rect.right,
                rect.bottom,
                ray.hit.distance,
                ray.hit.position.y,
                ray.hit.position.x))
        side = BoxSide::Bottom;

    switch (side) {
        case BoxSide::Right: ray.hit.normal = {1.0f, 0.0f}; break;
        case BoxSide::Left: ray.hit.normal = {-1.0f, 0.0f}; break;
        case BoxSide::Top: ray.hit.normal = {0.0f, 1.0f}; break;
        case BoxSide::Bottom: ray.hit.normal = {0.0f, -1.0f}; break;
        default: return false;
    }

    ray.hit.position = ray[ray.hit.distance];
    return true;
}


INLINE bool rightwardRayHitsBound(RayOf<vec2> &ray, const float x_bound) {
    f32 distance = x_bound - ray.origin.x;
    if (distance < 0 || // Rect is behind the ray
        distance > ray.direction.x) // Rect is too far in front of the ray
        return false;

    ray.hit.distance = distance;
    ray.hit.position = ray[distance / ray.direction.x];
    ray.hit.normal = {-1.0f, 0.0f};
    ray.hit.from_behind = false;

    return true;
}

INLINE bool leftwardRayHitsBound(RayOf<vec2> &ray, const float x_bound) {
    f32 distance = ray.origin.x - x_bound;
    if (distance < 0 || // Rect is behind the ray
        distance > -ray.direction.x) // Rect is too far in front of the ray
        return false;

    ray.hit.position = ray[distance / -ray.direction.x];
    ray.hit.normal = {1.0f, 0.0f};
    ray.hit.distance = distance;
    ray.hit.from_behind = false;

    return true;
}

INLINE bool horizontalRayHitsRect(RayOf<vec2> &ray, const Rect &rect) {
    if (ray.direction.x == 0) return false;
    return signbit(ray.direction.x) ?
           leftwardRayHitsBound(ray, rect.left) :
           rightwardRayHitsBound(ray, rect.right);
}

INLINE bool upwardRayHitsBound(RayOf<vec2> &ray, const float y_bound) {
    f32 distance = y_bound - ray.origin.y;
    if (distance < 0 || // Rect is behind the ray
        distance > ray.direction.y) // Rect is too far in front of the ray
        return false;

    ray.hit.distance = distance;
    ray.hit.position = ray[distance / ray.direction.y];
    ray.hit.normal = {0.0f, -1.0f};
    ray.hit.from_behind = false;

    return true;
}

INLINE bool downwardRayHitsBound(RayOf<vec2> &ray, const float y_bound) {
    f32 distance = ray.origin.y - y_bound;
    if (distance < 0 || // Rect is behind the ray
        distance > -ray.direction.y) // Rect is too far in front of the ray
        return false;

    ray.hit.position = ray[distance / -ray.direction.y];
    ray.hit.normal = {0.0f, 1.0f};
    ray.hit.distance = distance;
    ray.hit.from_behind = false;

    return true;
}

INLINE bool verticalRayHitsRect(RayOf<vec2> &ray, const Rect &rect) {
    if (ray.direction.y == 0) return false;
    return signbit(ray.direction.y) ?
           downwardRayHitsBound(ray, rect.bottom) :
           upwardRayHitsBound(ray, rect.top);
}

INLINE bool rayHitRect(RayOf<vec2> &ray, const Rect &rect) {
    if (ray.direction.x == 0) return verticalRayHitsRect(ray, rect);
    if (ray.direction.y == 0) return horizontalRayHitsRect(ray, rect);
    bool aiming_left{signbit(ray.direction.x)};
    bool aiming_down{signbit(ray.direction.y)};
    vec2 RD_rcp = 1.0f / ray.direction;
    vec2 t_near{aiming_left ? rect.right : rect.left, aiming_down ? rect.top : rect.bottom};
    vec2 t_far{ aiming_left ? rect.left : rect.right, aiming_down ? rect.bottom : rect.top};
    t_near -= ray.origin;
    t_far -= ray.origin;
    t_near *= RD_rcp;
    t_far *= RD_rcp;
    if (t_near.x > t_far.y || t_near.y > t_far.x) return false; // Early rejection

    f32 near_t = t_near.x > t_near.y ? t_near.x : t_near.y;
    f32 far_t  = t_far.x  < t_far.y  ? t_far.x  : t_far.y; if (far_t < 0) return false; // Reject if ray direction is pointing away from object

    // Contact point of collision from parametric line equation
    ray.hit.distance = near_t;
    ray.hit.position = ray[near_t];
    ray.hit.normal = 0.0f;
    if (     t_near.x > t_near.y) ray.hit.normal.x = aiming_left ? 1.0f : -1.0f;
    else if (t_near.x < t_near.y) ray.hit.normal.y = aiming_down ? 1.0f : -1.0f;

    return true;
}