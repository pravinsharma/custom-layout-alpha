#include "Keyboard.h"

namespace vkapp {

void Keyboard::beginFrame()
{
    m_previous = m_current;
}

void Keyboard::onKeyEvent(int key, int scancode, int action, int mods)
{
    if (key < 0 || key >= KeyCount) {
        return;
    }

    m_current[key] = (action == GLFW_PRESS || action == GLFW_REPEAT);
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

} // namespace vkapp
