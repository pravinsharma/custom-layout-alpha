#include "FontManager.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>

namespace vkapp::Graphics {

namespace {

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

} // namespace

FontManager::~FontManager() {
    shutdown();
}

bool FontManager::initialize(const std::string& assetsPath) {
    m_assetsPath = assetsPath;

    FT_Error err = FT_Init_FreeType(&m_library);
    if (err != 0) {
        std::cerr << "Failed to initialize FreeType\n";
        return false;
    }

    return true;
}

void FontManager::shutdown() {
    for (auto& font : m_fonts) {
        if (font.face) {
            FT_Done_Face(font.face);
            font.face = nullptr;
        }
    }
    m_fonts.clear();
    m_fontMap.clear();

    if (m_library) {
        FT_Done_FreeType(m_library);
        m_library = nullptr;
    }
}

std::string FontManager::resolveFontPath(const std::string& family, int weight) const {
    std::string lower = toLower(family);

    auto tryFile = [&](const std::string& file) -> std::string {
        std::vector<std::string> candidates = {
            m_assetsPath + file,
            "../" + m_assetsPath + file
        };
        for (const auto& path : candidates) {
            std::ifstream f(path);
            if (f.good()) return path;
        }
        return {};
    };

    if (lower.find("roboto condensed") != std::string::npos) {
        if (weight >= 700) {
            auto p = tryFile("RobotoCondensed-Bold.ttf");
            if (!p.empty()) return p;
        }
        auto p = tryFile("RobotoCondensed-Regular.ttf");
        if (!p.empty()) return p;
    }

    if (lower.find("roboto") != std::string::npos) {
        if (weight >= 700) {
            auto p = tryFile("Roboto-Bold.ttf");
            if (!p.empty()) return p;
        }
        if (weight <= 300) {
            auto p = tryFile("Roboto-Light.ttf");
            if (!p.empty()) return p;
        }
        if (weight >= 500 && weight < 700) {
            auto p = tryFile("Roboto-Medium.ttf");
            if (!p.empty()) return p;
        }
        auto p = tryFile("Roboto-Regular.ttf");
        if (!p.empty()) return p;
    }

    if (lower.find("open sans") != std::string::npos) {
        if (weight >= 800) {
            auto p = tryFile("OpenSans-ExtraBold.ttf");
            if (!p.empty()) return p;
        }
        if (weight >= 700) {
            auto p = tryFile("OpenSans-Bold.ttf");
            if (!p.empty()) return p;
        }
        if (weight >= 600) {
            auto p = tryFile("OpenSans-SemiBold.ttf");
            if (!p.empty()) return p;
        }
        if (weight <= 300) {
            auto p = tryFile("OpenSans-Light.ttf");
            if (!p.empty()) return p;
        }
        auto p = tryFile("OpenSans-Regular.ttf");
        if (!p.empty()) return p;
    }

    if (lower.find("times") != std::string::npos) {
        if (weight >= 700) {
            auto p = tryFile("timesbd.ttf");
            if (!p.empty()) return p;
        }
        auto p = tryFile("times.ttf");
        if (!p.empty()) return p;
    }

    if (lower.find("courier") != std::string::npos) {
        if (weight >= 700) {
            auto p = tryFile("courbd.ttf");
            if (!p.empty()) return p;
        }
        auto p = tryFile("cour.ttf");
        if (!p.empty()) return p;
    }

    if (lower.find("arial") != std::string::npos) {
        if (weight >= 700) {
            auto p = tryFile("arialbd.ttf");
            if (!p.empty()) return p;
        }
        auto p = tryFile("arial.ttf");
        if (!p.empty()) return p;
    }

    if (lower.find("calibri") != std::string::npos) {
        if (weight >= 700) {
            auto p = tryFile("calibrib.ttf");
            if (!p.empty()) return p;
        }
        auto p = tryFile("calibri.ttf");
        if (!p.empty()) return p;
    }

    if (lower.find("verdana") != std::string::npos) {
        if (weight >= 700) {
            auto p = tryFile("verdanab.ttf");
            if (!p.empty()) return p;
        }
        auto p = tryFile("verdana.ttf");
        if (!p.empty()) return p;
    }

    if (lower.find("segoe") != std::string::npos) {
        if (weight >= 700) {
            auto p = tryFile("segoeuib.ttf");
            if (!p.empty()) return p;
        }
        auto p = tryFile("segoeui.ttf");
        if (!p.empty()) return p;
    }

    if (lower.find("tahoma") != std::string::npos) {
        if (weight >= 700) {
            auto p = tryFile("tahomabd.ttf");
            if (!p.empty()) return p;
        }
        auto p = tryFile("tahoma.ttf");
        if (!p.empty()) return p;
    }

    if (lower == "sans-serif" || lower == "serif" || lower == "monospace") {
        auto p = tryFile("Roboto-Regular.ttf");
        if (!p.empty()) return p;
    }

    return {};
}

uint32_t FontManager::loadFont(const std::string& family, int weight) {
    auto key = std::make_pair(family, weight);
    auto it = m_fontMap.find(key);
    if (it != m_fontMap.end()) {
        return it->second;
    }

    std::string path = resolveFontPath(family, weight);
    if (path.empty()) {
        std::cerr << "Font not found: " << family << " weight=" << weight << "\n";
        return UINT32_MAX;
    }

    FT_Face face = nullptr;
    FT_Error err = FT_New_Face(m_library, path.c_str(), 0, &face);
    if (err != 0 || !face) {
        std::cerr << "Failed to load font: " << path << " (error=" << err << ")\n";
        return UINT32_MAX;
    }

    uint32_t id = static_cast<uint32_t>(m_fonts.size());
    FontFace ff;
    ff.face = face;
    ff.family = family;
    ff.weight = weight;
    m_fonts.push_back(ff);
    m_fontMap[key] = id;
    return id;
}

const FontFace* FontManager::getFont(uint32_t id) const {
    if (id >= m_fonts.size()) return nullptr;
    return &m_fonts[id];
}

uint32_t FontManager::getFontId(const std::string& family, int weight) {
    auto key = std::make_pair(family, weight);
    auto it = m_fontMap.find(key);
    if (it != m_fontMap.end()) {
        return it->second;
    }
    return loadFont(family, weight);
}

FT_Face FontManager::getFace(uint32_t fontId) const {
    if (fontId >= m_fonts.size()) return nullptr;
    return m_fonts[fontId].face;
}

float FontManager::getAscender(FT_Face face) const {
    if (!face) return 0.0f;
    float scale = face->size->metrics.y_scale / 65536.0f;
    return (face->ascender / 64.0f) * scale;
}

float FontManager::getDescender(FT_Face face) const {
    if (!face) return 0.0f;
    float scale = face->size->metrics.y_scale / 65536.0f;
    return (face->descender / 64.0f) * scale;
}

float FontManager::getLineHeight(FT_Face face) const {
    if (!face) return 0.0f;
    float scale = face->size->metrics.y_scale / 65536.0f;
    return (face->height / 64.0f) * scale;
}

} // namespace vkapp::Graphics
