cmake_minimum_required(VERSION 3.21)
project(SeriesDash360Xbox CXX)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(XdkXex)
add_xex(SeriesDash360 TYPE EXE
 SOURCES main_xbox.cpp Xbox360Platform.cpp XboxInput.cpp XboxRenderer.cpp
         ../../src/App.cpp ../../src/GameLibrary.cpp ../../src/FileManager.cpp ../../src/Settings.cpp
         ../../src/XexMetadata.cpp ../../src/DiscImporter.cpp ../../src/CoverService.cpp ../../src/PluginApi.cpp
 CONFIG "${CMAKE_CURRENT_SOURCE_DIR}/Application.xml"
 LIBRARIES d3d9 xinput
)
target_include_directories(SeriesDash360 PRIVATE ../../include .)
