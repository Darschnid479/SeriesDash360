cmake_minimum_required(VERSION 3.16)
project(SeriesDash360 VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(seriesdash_core
    src/App.cpp
    src/GameLibrary.cpp
    src/FileManager.cpp
    src/Settings.cpp
)
target_include_directories(seriesdash_core PUBLIC include)

add_executable(seriesdash360
    src/main.cpp
    platform/desktop/DesktopPlatform.cpp
)
target_include_directories(seriesdash360 PRIVATE include platform/desktop)
target_link_libraries(seriesdash360 PRIVATE seriesdash_core)

if (MSVC)
  target_compile_options(seriesdash_core PRIVATE /W4)
  target_compile_options(seriesdash360 PRIVATE /W4)
else()
  target_compile_options(seriesdash_core PRIVATE -Wall -Wextra -Wpedantic)
  target_compile_options(seriesdash360 PRIVATE -Wall -Wextra -Wpedantic)
endif()
