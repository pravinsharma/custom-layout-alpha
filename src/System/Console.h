#pragma once

#include <string>
#include <mutex>

namespace vkapp::System {

class Console {
public:
    static Console& get();

    template <typename... Args>
    void log(Args&&... args);
    template <typename... Args>
    void warn(Args&&... args);
    template <typename... Args>
    void error(Args&&... args);

private:
    Console() = default;
    void print(const std::string& message);
    std::mutex m_mutex;
};

template <typename... Args>
void Console::log(Args&&... args)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    print("LOG: ");
    ((print(std::to_string(args) + " ")), ...);
    print("\n");
}

template <typename... Args>
void Console::warn(Args&&... args)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    print("WARN: ");
    ((print(std::to_string(args) + " ")), ...);
    print("\n");
}

template <typename... Args>
void Console::error(Args&&... args)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    print("ERROR: ");
    ((print(std::to_string(args) + " ")), ...);
    print("\n");
}

} // namespace vkapp::System
