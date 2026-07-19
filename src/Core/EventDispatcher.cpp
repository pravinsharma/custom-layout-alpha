#include "EventDispatcher.h"

namespace vkapp::Core {

void EventDispatcher::addListener(Listener listener)
{
    m_listeners.push_back(std::move(listener));
}

void EventDispatcher::dispatch(const Event& event)
{
    for (auto it = m_listeners.rbegin(); it != m_listeners.rend(); ++it) {
        if ((*it)(event)) {
            break;
        }
    }
}

} // namespace vkapp::Core
