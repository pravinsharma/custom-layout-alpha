#include "Keyboard.h"

namespace vkapp::Input {

void Keyboard::beginFrame()
{
    m_previous = m_current;
}

void Keyboard::registerListeners(Core::EventDispatcher& dispatcher)
{
    dispatcher.addListener([this](const Core::Event& event) { return onEvent(event); });
}

bool Keyboard::onEvent(const Core::Event& event)
{
    if (!event.isInCategory(Core::EventCategory::Keyboard)) {
        return false;
    }

    switch (event.getType()) {
        case Core::EventType::KeyPress: {
            const auto& keyEvent = static_cast<const Core::KeyPressEvent&>(event);
            if (keyEvent.key >= 0 && keyEvent.key < KeyCount) {
                m_current[keyEvent.key] = true;
            }
            break;
        }
        case Core::EventType::KeyRelease: {
            const auto& keyEvent = static_cast<const Core::KeyReleaseEvent&>(event);
            if (keyEvent.key >= 0 && keyEvent.key < KeyCount) {
                m_current[keyEvent.key] = false;
            }
            break;
        }
        default:
            break;
    }

    return false;
}

bool Keyboard::isKeyDown(int key) const
{
    if (key < 0 || key >= KeyCount) {
        return false;
    }
    return m_current[key];
}

bool Keyboard::isKeyUp(int key) const
{
    if (key < 0 || key >= KeyCount) {
        return false;
    }
    return !m_current[key];
}

bool Keyboard::wasKeyPressed(int key) const
{
    if (key < 0 || key >= KeyCount) {
        return false;
    }
    return m_current[key] && !m_previous[key];
}

bool Keyboard::wasKeyReleased(int key) const
{
    if (key < 0 || key >= KeyCount) {
        return false;
    }
    return !m_current[key] && m_previous[key];
}

} // namespace vkapp::Input
