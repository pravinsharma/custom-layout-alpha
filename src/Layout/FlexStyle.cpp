#include "Layout/FlexStyle.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace vkapp::Layout {

namespace {

std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\n\r");
    return s.substr(start, end - start + 1);
}

std::string toLower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

float parseFloatValue(const std::string& s) {
    size_t i = 0;
    while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i;
    size_t j = s.size();
    while (j > i && std::isspace(static_cast<unsigned char>(s[j - 1]))) --j;
    std::string num = s.substr(i, j - i);

    if (num.size() > 2 && num.substr(num.size() - 2) == "px") {
        num = num.substr(0, num.size() - 2);
    }

    if (num.empty()) return 0.0f;
    return std::stof(num);
}

std::vector<std::string> splitValues(const std::string& value) {
    std::vector<std::string> parts;
    size_t start = 0;
    while (true) {
        size_t space = value.find(' ', start);
        if (space == std::string::npos) {
            std::string last = trim(value.substr(start));
            if (!last.empty()) parts.push_back(last);
            break;
        }
        std::string part = trim(value.substr(start, space - start));
        if (!part.empty()) parts.push_back(part);
        start = space + 1;
    }
    return parts;
}

void applyBoxShorthand(float& top, float& right, float& bottom, float& left, const std::string& value) {
    std::vector<std::string> parts = splitValues(value);
    if (parts.empty()) return;

    if (parts.size() == 1) {
        top = right = bottom = left = parseFloatValue(parts[0]);
    } else if (parts.size() == 2) {
        top = bottom = parseFloatValue(parts[0]);
        right = left = parseFloatValue(parts[1]);
    } else if (parts.size() == 3) {
        top = parseFloatValue(parts[0]);
        right = left = parseFloatValue(parts[1]);
        bottom = parseFloatValue(parts[2]);
    } else if (parts.size() >= 4) {
        top = parseFloatValue(parts[0]);
        right = parseFloatValue(parts[1]);
        bottom = parseFloatValue(parts[2]);
        left = parseFloatValue(parts[3]);
    }
}

}

void FlexStyle::parseStyle(const std::string& css) {
    style = css;
    applyStyle();
}

void FlexStyle::applyStyle() {
    if (style.empty()) return;

    std::vector<std::string> declarations;
    size_t start = 0;

    while (true) {
        size_t semicolon = style.find(';', start);
        if (semicolon == std::string::npos) {
            std::string last = style.substr(start);
            if (!trim(last).empty()) {
                declarations.push_back(trim(last));
            }
            break;
        }
        std::string decl = trim(style.substr(start, semicolon - start));
        if (!decl.empty()) {
            declarations.push_back(decl);
        }
        start = semicolon + 1;
    }

    for (const auto& decl : declarations) {
        size_t colon = decl.find(':');
        if (colon == std::string::npos) continue;

        std::string prop = trim(decl.substr(0, colon));
        std::string value = trim(decl.substr(colon + 1));

        std::string propLower = toLower(prop);
        std::string valueLower = toLower(value);

        if (propLower == "display") {
            if (valueLower == "flex") display = Display::Flex;
            else if (valueLower == "block") display = Display::Block;
            else if (valueLower == "none") display = Display::None;
            else if (valueLower == "grid") display = Display::Grid;
        } else if (propLower == "flex-direction") {
            if (valueLower == "row") direction = FlexDirection::Row;
            else if (valueLower == "row-reverse") direction = FlexDirection::RowReverse;
            else if (valueLower == "column") direction = FlexDirection::Column;
            else if (valueLower == "column-reverse") direction = FlexDirection::ColumnReverse;
        } else if (propLower == "flex-wrap") {
            if (valueLower == "nowrap") wrap = FlexWrap::NoWrap;
            else if (valueLower == "wrap") wrap = FlexWrap::Wrap;
            else if (valueLower == "wrap-reverse") wrap = FlexWrap::WrapReverse;
        } else if (propLower == "justify-content") {
            if (valueLower == "flex-start") justify = JustifyContent::FlexStart;
            else if (valueLower == "flex-end") justify = JustifyContent::FlexEnd;
            else if (valueLower == "center") justify = JustifyContent::Center;
            else if (valueLower == "space-between") justify = JustifyContent::SpaceBetween;
            else if (valueLower == "space-around") justify = JustifyContent::SpaceAround;
            else if (valueLower == "space-evenly") justify = JustifyContent::SpaceEvenly;
        } else if (propLower == "align-items") {
            if (valueLower == "stretch") alignItems = AlignItems::Stretch;
            else if (valueLower == "flex-start") alignItems = AlignItems::FlexStart;
            else if (valueLower == "flex-end") alignItems = AlignItems::FlexEnd;
            else if (valueLower == "center") alignItems = AlignItems::Center;
            else if (valueLower == "baseline") alignItems = AlignItems::Baseline;
        } else if (propLower == "align-content") {
            if (valueLower == "stretch") alignContent = AlignContent::Stretch;
            else if (valueLower == "flex-start") alignContent = AlignContent::FlexStart;
            else if (valueLower == "flex-end") alignContent = AlignContent::FlexEnd;
            else if (valueLower == "center") alignContent = AlignContent::Center;
            else if (valueLower == "space-between") alignContent = AlignContent::SpaceBetween;
            else if (valueLower == "space-around") alignContent = AlignContent::SpaceAround;
        } else if (propLower == "flex-grow") {
            flexGrow = parseFloatValue(value);
        } else if (propLower == "flex-shrink") {
            flexShrink = parseFloatValue(value);
        } else if (propLower == "flex-basis") {
            flexBasis = parseFloatValue(value);
        } else if (propLower == "gap") {
            float g = parseFloatValue(value);
            gapRow = g;
            gapColumn = g;
        } else if (propLower == "gap-row") {
            gapRow = parseFloatValue(value);
        } else if (propLower == "gap-column") {
            gapColumn = parseFloatValue(value);
        } else if (propLower == "margin") {
            applyBoxShorthand(marginTop, marginRight, marginBottom, marginLeft, value);
        } else if (propLower == "margin-top") {
            marginTop = parseFloatValue(value);
        } else if (propLower == "margin-right") {
            marginRight = parseFloatValue(value);
        } else if (propLower == "margin-bottom") {
            marginBottom = parseFloatValue(value);
        } else if (propLower == "margin-left") {
            marginLeft = parseFloatValue(value);
        } else if (propLower == "padding") {
            applyBoxShorthand(paddingTop, paddingRight, paddingBottom, paddingLeft, value);
        } else if (propLower == "padding-top") {
            paddingTop = parseFloatValue(value);
        } else if (propLower == "padding-right") {
            paddingRight = parseFloatValue(value);
        } else if (propLower == "padding-bottom") {
            paddingBottom = parseFloatValue(value);
        } else if (propLower == "padding-left") {
            paddingLeft = parseFloatValue(value);
        } else if (propLower == "border") {
            applyBoxShorthand(borderTop, borderRight, borderBottom, borderLeft, value);
        } else if (propLower == "border-top") {
            borderTop = parseFloatValue(value);
        } else if (propLower == "border-right") {
            borderRight = parseFloatValue(value);
        } else if (propLower == "border-bottom") {
            borderBottom = parseFloatValue(value);
        } else if (propLower == "border-left") {
            borderLeft = parseFloatValue(value);
        } else if (propLower == "min-width") {
            minWidth = parseFloatValue(value);
        } else if (propLower == "max-width") {
            maxWidth = parseFloatValue(value);
        } else if (propLower == "min-height") {
            minHeight = parseFloatValue(value);
        } else if (propLower == "max-height") {
            maxHeight = parseFloatValue(value);
        }
    }
}

} // namespace vkapp::Layout
