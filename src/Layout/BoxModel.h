#pragma once

#include "Rect.h"

namespace vkapp::Layout {

struct BoxModel {
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

    float marginHorizontal() const { return marginLeft + marginRight; }
    float marginVertical() const { return marginTop + marginBottom; }
    float paddingHorizontal() const { return paddingLeft + paddingRight; }
    float paddingVertical() const { return paddingTop + paddingBottom; }
    float borderHorizontal() const { return borderLeft + borderRight; }
    float borderVertical() const { return borderTop + borderBottom; }
};

} // namespace vkapp::Layout
