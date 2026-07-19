#pragma once

#include "Event.h"

#include <functional>
#include <vector>

namespace vkapp::Core {

class EventDispatcher {
public:
    using Listener = std::function<bool(const Event&)>;

    EventDispatcher() = default;
    ~EventDispatcher() = default;

    EventDispatcher(const EventDispatcher&) = delete;
    EventDispatcher& operator=(const EventDispatcher&) = delete;
    EventDispatcher(EventDispatcher&&) = delete;
    EventDispatcher& operator=(EventDispatcher&&) = delete;

    void addListener(Listener listener);
    void dispatch(const Event& event);

private:
    std::vector<Listener> m_listeners;
};

} // namespace vkapp::Core
