cmake_minimum_required(VERSION 3.10)

# Setting name of project
project(Chip-8-Emulator VERSION 1.0)

# setting C++ Standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Setting source files
set(SOURCE_FILES source/main.cpp source/chip8.cpp source/stack.cpp)

# Add executable
add_executable(CHIP8 ${SOURCE_FILES})

# Make sure SDL is included
INCLUDE(FindPkgConfig)
PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)

INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(CHIP8 ${SDL2_LIBRARIES})
