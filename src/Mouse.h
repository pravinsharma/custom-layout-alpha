#pragma once

#include <GLFW/glfw3.h>

#include <array>
#include <utility>

namespace vkapp {

class Mouse {
public:
    void beginFrame();
    void onButtonEvent(int button, int action, int mods);
    void onCursorPosition(double x, double y);
    void onScroll(double xOffset, double yOffset);

    bool isButtonDown(int button) const;
    bool isButtonUp(int button) const;
    bool wasButtonPressed(int button) const;
    bool wasButtonReleased(int button) const;

    std::pair<double, double> getPosition() const;
    std::pair<double, double> getScrollOffset() const;
    void resetScrollOffset();

private:
    static constexpr int ButtonCount = 8;
    std::array<bool, ButtonCount> m_currentButtons{};
    std::array<bool, ButtonCount> m_previousButtons{};

    double m_x = 0.0;
    double m_y = 0.0;
    double m_scrollX = 0.0;
    double m_scrollY = 0.0;
};

} // namespace vkapp
