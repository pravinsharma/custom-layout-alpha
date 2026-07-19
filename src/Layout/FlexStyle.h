#pragma once

#include <cstdint>

namespace vkapp::Layout {

enum class Display : uint32_t {
    None = 0,
    Block,
    Flex,
    Grid
};

enum class FlexDirection : uint32_t {
    Row = 0,
    RowReverse,
    Column,
    ColumnReverse
};

enum class FlexWrap : uint32_t {
    NoWrap = 0,
    Wrap,
    WrapReverse
};

enum class JustifyContent : uint32_t {
    FlexStart = 0,
    FlexEnd,
    Center,
    SpaceBetween,
    SpaceAround,
    SpaceEvenly
};

enum class AlignItems : uint32_t {
    Stretch = 0,
    FlexStart,
    FlexEnd,
    Center,
    Baseline
};

enum class AlignContent : uint32_t {
    Stretch = 0,
    FlexStart,
    FlexEnd,
    Center,
    SpaceBetween,
    SpaceAround
};

struct FlexStyle {
    Display display = Display::Flex;
    FlexDirection direction = FlexDirection::Row;
    FlexWrap wrap = FlexWrap::NoWrap;
    JustifyContent justify = JustifyContent::FlexStart;
    AlignItems alignItems = AlignItems::Stretch;
    AlignContent alignContent = AlignContent::Stretch;

    float flexGrow = 0.0f;
    float flexShrink = 1.0f;
    float flexBasis = 0.0f;

    float gapRow = 0.0f;
    float gapColumn = 0.0f;

    float minWidth = 0.0f;
    float maxWidth = 0.0f;
    float minHeight = 0.0f;
    float maxHeight = 0.0f;
};

} // namespace vkapp::Layout
