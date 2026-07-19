#include "Mouse.h"

namespace vkapp {

void Mouse::beginFrame()
{
    m_previousButtons = m_currentButtons;
    m_scrollX = 0.0;
    m_scrollY = 0.0;
}

void Mouse::onButtonEvent(int button, int action, int mods)
{
    if (button < 0 || button >= ButtonCount) {
        return;
    }

    m_currentButtons[button] = (action == GLFW_PRESS);
}

void Mouse::onCursorPosition(double x, double y)
{
    m_x = x;
    m_y = y;
}

void Mouse::onScroll(double xOffset, double yOffset)
{
    m_scrollX += xOffset;
    m_scrollY += yOffset;
}

bool Mouse::isButtonDown(int button) const
{
    if (button < 0 || button >= ButtonCount) {
        return false;
    }
    return m_currentButtons[button];
}

bool Mouse::isButtonUp(int button) const
{
    if (button < 0 || button >= ButtonCount) {
        return false;
    }
    return !m_currentButtons[button];
}

bool Mouse::wasButtonPressed(int button) const
{
    if (button < 0 || button >= ButtonCount) {
        return false;
    }
    return m_currentButtons[button] && !m_previousButtons[button];
}

bool Mouse::wasButtonReleased(int button) const
{
    if (button < 0 || button >= ButtonCount) {
        return false;
    }
    return !m_currentButtons[button] && m_previousButtons[button];
}

std::pair<double, double> Mouse::getPosition() const
{
    return { m_x, m_y };
}

std::pair<double, double> Mouse::getScrollOffset() const
{
    return { m_scrollX, m_scrollY };
}

void Mouse::resetScrollOffset()
{
    m_scrollX = 0.0;
    m_scrollY = 0.0;
}

} // namespace vkapp
