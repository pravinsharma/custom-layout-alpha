#include "TextLayout.h"

#include "FontManager.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <vector>

namespace vkapp::Graphics {

namespace {

std::vector<std::string> splitWords(const std::string& text) {
    std::vector<std::string> words;
    std::istringstream iss(text);
    std::string word;
    while (iss >> word) {
        words.push_back(word);
    }
    return words;
}

} // namespace

TextLayout TextLayoutEngine::layout(const std::string& text, uint32_t fontId, float fontSize,
                                     float maxWidth, TextAlign align, FontManager* fontManager,
                                     GlyphAtlas* atlas) {
    TextLayout result{};

    if (text.empty() || !fontManager || !atlas) return result;

    FT_Face face = fontManager->getFace(fontId);
    if (!face) return result;

    FT_Set_Char_Size(face, 0, static_cast<int>(fontSize * 64), 96, 96);

    float ascender = fontManager->getAscender(face);
    float descender = fontManager->getDescender(face);
    float lineHeight = fontManager->getLineHeight(face);
    if (lineHeight < fontSize) lineHeight = fontSize;

    float spaceAdvance = fontSize * 0.3f;

    struct Line {
        std::vector<GlyphInstance> glyphs;
        float width = 0.0f;
    };

    std::vector<Line> lines;
    Line currentLine;
    float currentX = 0.0f;
    float currentY = ascender;

    std::vector<std::string> words = splitWords(text);

    for (size_t w = 0; w < words.size(); ++w) {
        const std::string& word = words[w];
        float wordWidth = 0.0f;

        for (char c : word) {
            uint32_t codepoint = static_cast<uint8_t>(c);
            const Glyph* glyph = atlas->getGlyph(fontId, codepoint, fontManager);
            if (glyph) {
                wordWidth += glyph->advance;
            }
        }

        bool fits = (currentLine.width + (currentLine.glyphs.empty() ? 0.0f : spaceAdvance) + wordWidth) <= maxWidth || maxWidth <= 0.0f;

        if (!fits && !currentLine.glyphs.empty()) {
            lines.push_back(currentLine);
            currentLine = Line{};
            currentLine.width = wordWidth;
            currentY += lineHeight;

            float gx = 0.0f;
            float gy = currentY - ascender + ascender;
            for (char c : word) {
                uint32_t codepoint = static_cast<uint8_t>(c);
                const Glyph* glyph = atlas->getGlyph(fontId, codepoint, fontManager);
                if (!glyph) continue;

                GlyphInstance instance{};
                instance.x = gx + glyph->bearingX;
                instance.y = gy - glyph->bearingY;
                instance.width = glyph->width;
                instance.height = glyph->height;
                instance.u0 = glyph->u0;
                instance.v0 = glyph->v0;
                instance.u1 = glyph->u1;
                instance.v1 = glyph->v1;
                currentLine.glyphs.push_back(instance);
                gx += glyph->advance;
            }
        } else {
            if (!currentLine.glyphs.empty()) {
                currentLine.width += spaceAdvance;
            }
            currentLine.width += wordWidth;

            float gx = currentLine.width - wordWidth;
            float gy = currentY;
            for (char c : word) {
                uint32_t codepoint = static_cast<uint8_t>(c);
                const Glyph* glyph = atlas->getGlyph(fontId, codepoint, fontManager);
                if (!glyph) continue;

                GlyphInstance instance{};
                instance.x = gx + glyph->bearingX;
                instance.y = gy - glyph->bearingY;
                instance.width = glyph->width;
                instance.height = glyph->height;
                instance.u0 = glyph->u0;
                instance.v0 = glyph->v0;
                instance.u1 = glyph->u1;
                instance.v1 = glyph->v1;
                currentLine.glyphs.push_back(instance);
                gx += glyph->advance;
            }
        }
    }

    if (!currentLine.glyphs.empty()) {
        lines.push_back(currentLine);
    }

    if (lines.empty() && !words.empty()) {
        Line line;
        float gx = 0.0f;
        float gy = currentY;
        for (const auto& word : words) {
            for (char c : word) {
                uint32_t codepoint = static_cast<uint8_t>(c);
                const Glyph* glyph = atlas->getGlyph(fontId, codepoint, fontManager);
                if (!glyph) continue;

                GlyphInstance instance{};
                instance.x = gx + glyph->bearingX;
                instance.y = gy - glyph->bearingY;
                instance.width = glyph->width;
                instance.height = glyph->height;
                instance.u0 = glyph->u0;
                instance.v0 = glyph->v0;
                instance.u1 = glyph->u1;
                instance.v1 = glyph->v1;
                line.glyphs.push_back(instance);
                gx += glyph->advance;
            }
        }
        line.width = gx;
        lines.push_back(line);
    }

    float maxLineWidth = 0.0f;
    for (const auto& line : lines) {
        if (line.width > maxLineWidth) {
            maxLineWidth = line.width;
        }
    }

    float totalHeight = lines.size() * lineHeight;

    for (auto& line : lines) {
        float offsetX = 0.0f;
        if (align == TextAlign::Center && maxWidth > 0.0f) {
            offsetX = (maxWidth - line.width) * 0.5f;
        } else if (align == TextAlign::Right && maxWidth > 0.0f) {
            offsetX = maxWidth - line.width;
        }

        for (auto& g : line.glyphs) {
            g.x += offsetX;
            result.glyphs.push_back(g);
        }
    }

    result.width = maxWidth > 0.0f ? maxWidth : maxLineWidth;
    result.height = totalHeight;

    return result;
}

} // namespace vkapp::Graphics
