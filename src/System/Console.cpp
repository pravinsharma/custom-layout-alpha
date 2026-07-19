#include "Console.h"

#include <iostream>

namespace vkapp::System {

Console& Console::get()
{
    static Console instance;
    return instance;
}

void Console::print(const std::string& message)
{
    std::cout << message;
}

} // namespace vkapp::System
