cmake_minimum_required(VERSION 3.15)
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
set(CMAKE_GENERATOR "Ninja" CACHE STRING "")

project(VulkanApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(DEFINED ENV{VULKAN_SDK})
    set(Vulkan_INCLUDE_DIR "$ENV{VULKAN_SDK}/Include")
    find_library(Vulkan_LIBRARY NAMES vulkan-1 PATHS "$ENV{VULKAN_SDK}/Lib" "$ENV{VULKAN_SDK}/Lib32")
    if(NOT Vulkan_LIBRARY)
        message(FATAL_ERROR "Vulkan library not found in $ENV{VULKAN_SDK}/Lib or /Lib32")
    endif()
else()
    find_package(Vulkan REQUIRED)
endif()

find_package(glfw3 CONFIG REQUIRED)

add_executable(${PROJECT_NAME}
    src/main.cpp
    src/Core/Window.cpp
    src/Core/EventDispatcher.cpp
    src/Input/Keyboard.cpp
    src/Input/Mouse.cpp
    src/System/Console.cpp
    src/Layout/FlexLayoutEngine.cpp
    src/Layout/LayoutDumper.cpp
    src/Layout/FlexTests.cpp
    src/Graphics/RenderCommandBuilder.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw ${Vulkan_LIBRARY})
