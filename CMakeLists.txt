cmake_minimum_required(VERSION 3.15)
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

project(VulkanApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

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
find_package(freetype CONFIG REQUIRED)

set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/Graphics/shaders)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${SHADER_BINARY_DIR})

set(VERT_SHADER ${SHADER_SOURCE_DIR}/quad.vert)
set(FRAG_SHADER ${SHADER_SOURCE_DIR}/quad.frag)
set(TEXT_VERT_SHADER ${SHADER_SOURCE_DIR}/text.vert)
set(TEXT_FRAG_SHADER ${SHADER_SOURCE_DIR}/text.frag)
set(VERT_SPV ${SHADER_BINARY_DIR}/quad.vert.spv)
set(FRAG_SPV ${SHADER_BINARY_DIR}/quad.frag.spv)
set(TEXT_VERT_SPV ${SHADER_BINARY_DIR}/text.vert.spv)
set(TEXT_FRAG_SPV ${SHADER_BINARY_DIR}/text.frag.spv)

add_custom_command(
    OUTPUT ${VERT_SPV} ${FRAG_SPV} ${TEXT_VERT_SPV} ${TEXT_FRAG_SPV}
    COMMAND $ENV{VULKAN_SDK}/Bin/glslangValidator.exe -V ${VERT_SHADER} -o ${VERT_SPV}
    COMMAND $ENV{VULKAN_SDK}/Bin/glslangValidator.exe -V ${FRAG_SHADER} -o ${FRAG_SPV}
    COMMAND $ENV{VULKAN_SDK}/Bin/glslangValidator.exe -V ${TEXT_VERT_SHADER} -o ${TEXT_VERT_SPV}
    COMMAND $ENV{VULKAN_SDK}/Bin/glslangValidator.exe -V ${TEXT_FRAG_SHADER} -o ${TEXT_FRAG_SPV}
    DEPENDS ${VERT_SHADER} ${FRAG_SHADER} ${TEXT_VERT_SHADER} ${TEXT_FRAG_SHADER}
    COMMENT "Compiling GLSL shaders to SPIR-V"
)

add_custom_target(Shaders ALL DEPENDS ${VERT_SPV} ${FRAG_SPV} ${TEXT_VERT_SPV} ${TEXT_FRAG_SPV})

set(FLEX_APP_SOURCES
    src/Core/Window.cpp
    src/Core/EventDispatcher.cpp
    src/Input/Keyboard.cpp
    src/Input/Mouse.cpp
    src/System/Console.cpp
    src/Layout/FlexStyle.cpp
    src/Layout/FlexLayoutEngine.cpp
    src/Layout/LayoutDumper.cpp
    src/Layout/FlexTests.cpp
    src/Layout/HtmlParser.cpp
    src/Graphics/RenderCommandBuilder.cpp
    src/Graphics/VulkanRenderer.cpp
    src/Graphics/FontManager.cpp
    src/Graphics/GlyphAtlas.cpp
    src/Graphics/TextLayout.cpp
    src/Graphics/TextRenderer.cpp
)

add_library(flex_layout_lib STATIC ${FLEX_APP_SOURCES})
target_include_directories(flex_layout_lib PUBLIC
    ${Vulkan_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_link_libraries(flex_layout_lib PUBLIC glfw ${Vulkan_LIBRARY} Freetype::Freetype)
target_compile_definitions(flex_layout_lib PUBLIC SHADER_BINARY_DIR="${SHADER_BINARY_DIR}")

add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE flex_layout_lib)
add_dependencies(${PROJECT_NAME} Shaders)

add_subdirectory(examples)
