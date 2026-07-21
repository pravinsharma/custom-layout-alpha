#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <utility>

namespace vkapp::Graphics {

struct FontFace {
    FT_Face face = nullptr;
    std::string family;
    int weight = 400;
};

class FontManager {
public:
    FontManager() = default;
    ~FontManager();

    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;
    FontManager(FontManager&&) = delete;
    FontManager& operator=(FontManager&&) = delete;

    bool initialize(const std::string& assetsPath = "assets/fonts/");
    void shutdown();

    uint32_t loadFont(const std::string& family, int weight);
    const FontFace* getFont(uint32_t id) const;
    uint32_t getFontId(const std::string& family, int weight);
    FT_Face getFace(uint32_t fontId) const;
    float getAscender(FT_Face face) const;
    float getDescender(FT_Face face) const;
    float getLineHeight(FT_Face face) const;

private:
    std::string resolveFontPath(const std::string& family, int weight) const;

    FT_Library m_library = nullptr;
    std::vector<FontFace> m_fonts;
    std::map<std::pair<std::string, int>, uint32_t> m_fontMap;
    std::string m_assetsPath;
};

} // namespace vkapp::Graphics
