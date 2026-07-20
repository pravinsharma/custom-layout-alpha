#include "HtmlParser.h"

#include "Layout/LayoutNode.h"

#include <algorithm>
#include <cctype>
#include <functional>
#include <memory>
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

} // namespace

class HtmlParser {
public:
    HtmlTree parse(const std::string& html) {
        m_html = html;
        m_pos = 0;
        LayoutNode* root = parseNode();
        return HtmlTree(root, HtmlTreeDeleter{});
    }

private:
    std::string m_html;
    size_t m_pos = 0;

    void skipWhitespace() {
        while (m_pos < m_html.size() && std::isspace(static_cast<unsigned char>(m_html[m_pos]))) {
            ++m_pos;
        }
    }

    LayoutNode* parseNode() {
        skipWhitespace();

        if (m_pos >= m_html.size() || m_html[m_pos] != '<') {
            size_t start = m_pos;
            while (m_pos < m_html.size() && m_html[m_pos] != '<') ++m_pos;
            std::string text = trim(m_html.substr(start, m_pos - start));
            if (!text.empty()) {
                return new LayoutNode(text);
            }
            return nullptr;
        }

        ++m_pos;
        size_t tagStart = m_pos;
        while (m_pos < m_html.size() && m_html[m_pos] != '>' && m_html[m_pos] != ' ' && m_html[m_pos] != '\t') ++m_pos;
        std::string tagName = m_html.substr(tagStart, m_pos - tagStart);

        if (!tagName.empty() && tagName[0] == '/') {
            return nullptr;
        }

        std::vector<std::pair<std::string, std::string>> attrs;
        while (m_pos < m_html.size() && m_html[m_pos] != '>') {
            skipWhitespace();
            if (m_pos >= m_html.size() || m_html[m_pos] == '>') break;

            size_t attrStart = m_pos;
            while (m_pos < m_html.size() && m_html[m_pos] != '=' && m_html[m_pos] != ' ' && m_html[m_pos] != '>' && m_html[m_pos] != '\t') ++m_pos;
            std::string attrName = m_html.substr(attrStart, m_pos - attrStart);

            std::string attrValue;
            if (m_pos < m_html.size() && m_html[m_pos] == '=') {
                ++m_pos;
                skipWhitespace();
                if (m_pos < m_html.size() && (m_html[m_pos] == '"' || m_html[m_pos] == '\'')) {
                    char quote = m_html[m_pos++];
                    size_t valStart = m_pos;
                    while (m_pos < m_html.size() && m_html[m_pos] != quote) ++m_pos;
                    attrValue = m_html.substr(valStart, m_pos - valStart);
                    if (m_pos < m_html.size()) ++m_pos;
                }
            }

            attrs.emplace_back(attrName, attrValue);
        }

        bool selfClosing = false;
        if (m_pos > tagStart) {
            size_t checkPos = m_pos;
            while (checkPos > tagStart && std::isspace(static_cast<unsigned char>(m_html[checkPos - 1]))) --checkPos;
            if (checkPos > tagStart && m_html[checkPos - 1] == '/') {
                selfClosing = true;
            }
        }

        if (m_pos < m_html.size() && m_html[m_pos] == '>') ++m_pos;

        auto* node = new LayoutNode(tagName);

        std::string styleStr;
        std::string className;
        for (auto& [name, value] : attrs) {
            std::string nameLower = toLower(name);
            if (nameLower == "class") {
                className = value;
            } else if (nameLower == "style") {
                styleStr = value;
            } else if (nameLower == "width") {
                node->explicitWidth = std::stof(value);
                node->hasExplicitWidth = true;
            } else if (nameLower == "height") {
                node->explicitHeight = std::stof(value);
                node->hasExplicitHeight = true;
            }
        }

        if (!className.empty()) {
            node->name = className;
        }

        if (!styleStr.empty()) {
            node->inlineStyle = styleStr;
        }

        if (!selfClosing) {
            while (m_pos < m_html.size()) {
                skipWhitespace();
                if (m_pos + 1 < m_html.size() && m_html[m_pos] == '<' && m_html[m_pos + 1] == '/') {
                    break;
                }
                if (m_pos >= m_html.size() || m_html[m_pos] != '<') {
                    size_t start = m_pos;
                    while (m_pos < m_html.size() && m_html[m_pos] != '<') ++m_pos;
                    std::string text = trim(m_html.substr(start, m_pos - start));
                    if (!text.empty() && !node->isFlexContainer && node->name.empty()) {
                        node->name = text;
                    }
                    continue;
                }
                auto* child = parseNode();
                if (child) {
                    node->addChild(child);
                }
            }
        }

        if (m_pos + 1 < m_html.size() && m_html[m_pos] == '<' && m_html[m_pos + 1] == '/') {
            m_pos += 2;
            while (m_pos < m_html.size() && m_html[m_pos] != '>') ++m_pos;
            if (m_pos < m_html.size()) ++m_pos;
        }

        return node;
    }
};

HtmlTree parseHtml(const std::string& html) {
    HtmlParser parser;
    return parser.parse(html);
}

Stylesheet parseCss(const std::string& css) {
    Stylesheet result;

    size_t pos = 0;
    while (pos < css.size()) {
        size_t openBrace = css.find('{', pos);
        if (openBrace == std::string::npos) break;

        size_t closeBrace = css.find('}', openBrace);
        if (closeBrace == std::string::npos) break;

        std::string selector = trim(css.substr(pos, openBrace - pos));
        std::string body = trim(css.substr(openBrace + 1, closeBrace - openBrace - 1));

        if (!selector.empty() && !body.empty()) {
            std::string className = selector;
            if (!className.empty() && className[0] == '.') {
                className = className.substr(1);
            }
            result[className] = body;
        }

        pos = closeBrace + 1;
    }

    return result;
}

void applyCss(LayoutNode& root, const Stylesheet& stylesheet) {
    std::function<void(LayoutNode&)> apply = [&](LayoutNode& node) {
        if (!node.name.empty()) {
            size_t start = 0;
            size_t end = node.name.find(' ');
            if (end == std::string::npos) {
                auto it = stylesheet.find(node.name);
                if (it != stylesheet.end()) {
                    node.flex.parseStyle(it->second);
                }
            } else {
                while (start < node.name.size()) {
                    end = node.name.find(' ', start);
                    if (end == std::string::npos) end = node.name.size();
                    std::string className = node.name.substr(start, end - start);
                    auto it = stylesheet.find(className);
                    if (it != stylesheet.end()) {
                        node.flex.parseStyle(it->second);
                    }
                    start = end + 1;
                }
            }
        }

        if (node.flex.cssWidth > 0.0f) {
            node.explicitWidth = node.flex.cssWidth;
            node.hasExplicitWidth = true;
        }
        if (node.flex.cssHeight > 0.0f) {
            node.explicitHeight = node.flex.cssHeight;
            node.hasExplicitHeight = true;
        }

        if (node.flex.display == Display::Flex || node.flex.display == Display::Block) {
            node.isFlexContainer = true;
        }

        node.overflow = node.flex.overflow;

        if (!node.inlineStyle.empty()) {
            node.flex.parseStyle(node.inlineStyle);
            if (node.flex.cssWidth > 0.0f) {
                node.explicitWidth = node.flex.cssWidth;
                node.hasExplicitWidth = true;
            }
            if (node.flex.cssHeight > 0.0f) {
                node.explicitHeight = node.flex.cssHeight;
                node.hasExplicitHeight = true;
            }

            if (node.flex.display == Display::Flex || node.flex.display == Display::Block) {
                node.isFlexContainer = true;
            }
        }

        for (auto* child : node.children) {
            apply(*child);
        }
    };

    apply(root);
}

} // namespace vkapp::Layout
