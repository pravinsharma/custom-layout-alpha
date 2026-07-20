#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "../Graphics/RenderCommand.h"

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

    float marginTop = 0.0f;
    float marginRight = 0.0f;
    float marginBottom = 0.0f;
    float marginLeft = 0.0f;

    float paddingTop = 0.0f;
    float paddingRight = 0.0f;
    float paddingBottom = 0.0f;
    float paddingLeft = 0.0f;

    float borderTop = 0.0f;
    float borderRight = 0.0f;
    float borderBottom = 0.0f;
    float borderLeft = 0.0f;

    std::optional<Graphics::Color> backgroundColor;
    std::optional<Graphics::Color> borderColor;

    float minWidth = 0.0f;
    float maxWidth = FLT_MAX;
    float minHeight = 0.0f;
    float maxHeight = FLT_MAX;

    std::string style;

    void parseStyle(const std::string& css);
    void applyStyle();
};

} // namespace vkapp::Layout
