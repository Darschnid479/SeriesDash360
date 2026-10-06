cmake_minimum_required(VERSION 3.21)
project(SeriesDash360 VERSION 1.0.0 LANGUAGES C CXX)

if(CMAKE_SYSTEM_NAME STREQUAL "Generic" AND CMAKE_SYSTEM_PROCESSOR STREQUAL "ppcbe")
  add_subdirectory(platform/xbox360)
else()
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  add_library(seriesdash_core
    src/App.cpp src/GameLibrary.cpp src/FileManager.cpp src/Settings.cpp
    src/XexMetadata.cpp src/DiscImporter.cpp src/CoverService.cpp src/PluginApi.cpp)
  target_include_directories(seriesdash_core PUBLIC include)
  add_executable(seriesdash360 src/main.cpp platform/desktop/DesktopPlatform.cpp)
  target_include_directories(seriesdash360 PRIVATE include platform/desktop)
  target_link_libraries(seriesdash360 PRIVATE seriesdash_core)
endif()
