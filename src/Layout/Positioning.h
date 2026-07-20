#pragma once

namespace vkapp::Layout {

enum class Position {
    Static = 0,
    Relative,
    Absolute,
    Fixed,
    Sticky
};

struct Positioning {
    Position type = Position::Static;
    float top = 0.0f;
    float right = 0.0f;
    float bottom = 0.0f;
    float left = 0.0f;
    int zIndex = 0;
};

} // namespace vkapp::Layout
