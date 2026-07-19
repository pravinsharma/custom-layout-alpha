#pragma once

#include <cstdint>
#include <string>
#include <variant>

namespace vkapp::Core {

enum class EventType : uint32_t {
    None = 0,
    KeyPress,
    KeyRelease,
    MouseButtonPress,
    MouseButtonRelease,
    MouseMove,
    MouseScroll,
    WindowResize,
    WindowClose
};

enum class EventCategory : uint32_t {
    None = 0,
    Input = 1 << 0,
    Keyboard = 1 << 1,
    Mouse = 1 << 2,
    Window = 1 << 3
};

class Event {
public:
    virtual ~Event() = default;
    virtual EventType getType() const = 0;
    virtual EventCategory getCategory() const = 0;
    virtual std::string toString() const { return "Event"; }
    bool isInCategory(EventCategory category) const
    {
        return (static_cast<uint32_t>(getCategory()) & static_cast<uint32_t>(category)) != 0;
    }
};

class KeyEvent : public Event {
public:
    int key;
    int scancode;
    int mods;

    KeyEvent(int key, int scancode, int mods)
        : key(key)
        , scancode(scancode)
        , mods(mods)
    {
    }
};

class KeyPressEvent : public KeyEvent {
public:
    using KeyEvent::KeyEvent;
    EventType getType() const override { return EventType::KeyPress; }
    EventCategory getCategory() const override
    {
        return static_cast<EventCategory>(static_cast<uint32_t>(EventCategory::Keyboard) | static_cast<uint32_t>(EventCategory::Input));
    }
};

class KeyReleaseEvent : public KeyEvent {
public:
    using KeyEvent::KeyEvent;
    EventType getType() const override { return EventType::KeyRelease; }
    EventCategory getCategory() const override
    {
        return static_cast<EventCategory>(static_cast<uint32_t>(EventCategory::Keyboard) | static_cast<uint32_t>(EventCategory::Input));
    }
};

class MouseButtonEvent : public Event {
public:
    int button;
    int mods;
    double x;
    double y;

    MouseButtonEvent(int button, int mods, double x, double y)
        : button(button)
        , mods(mods)
        , x(x)
        , y(y)
    {
    }
};

class MouseButtonPressEvent : public MouseButtonEvent {
public:
    using MouseButtonEvent::MouseButtonEvent;
    EventType getType() const override { return EventType::MouseButtonPress; }
    EventCategory getCategory() const override
    {
        return static_cast<EventCategory>(static_cast<uint32_t>(EventCategory::Mouse) | static_cast<uint32_t>(EventCategory::Input));
    }
};

class MouseButtonReleaseEvent : public MouseButtonEvent {
public:
    using MouseButtonEvent::MouseButtonEvent;
    EventType getType() const override { return EventType::MouseButtonRelease; }
    EventCategory getCategory() const override
    {
        return static_cast<EventCategory>(static_cast<uint32_t>(EventCategory::Mouse) | static_cast<uint32_t>(EventCategory::Input));
    }
};

class MouseMoveEvent : public Event {
public:
    double x;
    double y;

    MouseMoveEvent(double x, double y)
        : x(x)
        , y(y)
    {
    }

    EventType getType() const override { return EventType::MouseMove; }
    EventCategory getCategory() const override
    {
        return static_cast<EventCategory>(static_cast<uint32_t>(EventCategory::Mouse) | static_cast<uint32_t>(EventCategory::Input));
    }
};

class MouseScrollEvent : public Event {
public:
    double xOffset;
    double yOffset;

    MouseScrollEvent(double xOffset, double yOffset)
        : xOffset(xOffset)
        , yOffset(yOffset)
    {
    }

    EventType getType() const override { return EventType::MouseScroll; }
    EventCategory getCategory() const override
    {
        return static_cast<EventCategory>(static_cast<uint32_t>(EventCategory::Mouse) | static_cast<uint32_t>(EventCategory::Input));
    }
};

class WindowResizeEvent : public Event {
public:
    uint32_t width;
    uint32_t height;

    WindowResizeEvent(uint32_t width, uint32_t height)
        : width(width)
        , height(height)
    {
    }

    EventType getType() const override { return EventType::WindowResize; }
    EventCategory getCategory() const override { return EventCategory::Window; }
};

class WindowCloseEvent : public Event {
public:
    EventType getType() const override { return EventType::WindowClose; }
    EventCategory getCategory() const override { return EventCategory::Window; }
};

} // namespace vkapp::Core
