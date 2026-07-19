#include "Mouse.h"

namespace vkapp::Input {

void Mouse::beginFrame()
{
    m_previousButtons = m_currentButtons;
    m_scrollX = 0.0;
    m_scrollY = 0.0;
}

void Mouse::registerListeners(Core::EventDispatcher& dispatcher)
{
    dispatcher.addListener([this](const Core::Event& event) { return onEvent(event); });
}

bool Mouse::onEvent(const Core::Event& event)
{
    if (!event.isInCategory(Core::EventCategory::Mouse)) {
        return false;
    }

    switch (event.getType()) {
        case Core::EventType::MouseButtonPress: {
            const auto& btnEvent = static_cast<const Core::MouseButtonPressEvent&>(event);
            if (btnEvent.button >= 0 && btnEvent.button < ButtonCount) {
                m_currentButtons[btnEvent.button] = true;
                m_x = btnEvent.x;
                m_y = btnEvent.y;
            }
            break;
        }
        case Core::EventType::MouseButtonRelease: {
            const auto& btnEvent = static_cast<const Core::MouseButtonReleaseEvent&>(event);
            if (btnEvent.button >= 0 && btnEvent.button < ButtonCount) {
                m_currentButtons[btnEvent.button] = false;
                m_x = btnEvent.x;
                m_y = btnEvent.y;
            }
            break;
        }
        case Core::EventType::MouseMove: {
            const auto& moveEvent = static_cast<const Core::MouseMoveEvent&>(event);
            m_x = moveEvent.x;
            m_y = moveEvent.y;
            break;
        }
        case Core::EventType::MouseScroll: {
            const auto& scrollEvent = static_cast<const Core::MouseScrollEvent&>(event);
            m_scrollX += scrollEvent.xOffset;
            m_scrollY += scrollEvent.yOffset;
            break;
        }
        default:
            break;
    }

    return false;
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

} // namespace vkapp::Input
