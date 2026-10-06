list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(XdkXex)

add_xex(SeriesDash360 TYPE EXE
 SOURCES
   NativeMain.cpp
   NativeRuntime.cpp
   NativeRenderer.cpp
   NativeNetwork.cpp
   NativeState.cpp
   NativeTools.cpp
   NativeExtensions.cpp
 CONFIG "${CMAKE_CURRENT_SOURCE_DIR}/Application.xml"
 LIBRARIES d3d9 d3dx9 xinput xhttp)

target_include_directories(SeriesDash360 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
