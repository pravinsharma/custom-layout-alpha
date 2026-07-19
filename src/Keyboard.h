#pragma once

#include <GLFW/glfw3.h>

#include <array>

namespace vkapp {

class Keyboard {
public:
    void beginFrame();
    void onKeyEvent(int key, int scancode, int action, int mods);

    bool isKeyDown(int key) const;
    bool isKeyUp(int key) const;
    bool wasKeyPressed(int key) const;
    bool wasKeyReleased(int key) const;

private:
    static constexpr int KeyCount = 512;
    std::array<bool, KeyCount> m_current{};
    std::array<bool, KeyCount> m_previous{};
};

} // namespace vkapp
