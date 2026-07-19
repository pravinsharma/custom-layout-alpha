#pragma once

#include "../Core/Event.h"
#include "../Core/EventDispatcher.h"

#include <array>

namespace vkapp::Input {

class Keyboard {
public:
    void beginFrame();
    void registerListeners(Core::EventDispatcher& dispatcher);

    bool isKeyDown(int key) const;
    bool isKeyUp(int key) const;
    bool wasKeyPressed(int key) const;
    bool wasKeyReleased(int key) const;

private:
    bool onEvent(const Core::Event& event);

    static constexpr int KeyCount = 512;
    std::array<bool, KeyCount> m_current{};
    std::array<bool, KeyCount> m_previous{};
};

} // namespace vkapp::Input
