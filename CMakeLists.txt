cmake_minimum_required(VERSION 3.25)
project(lantorch LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

file(GLOB_RECURSE ALL_SOURCES ${ALL_SOURCES} src/*)
add_executable(${PROJECT_NAME} ${ALL_SOURCES})
include_directories(src)

# pthread (sometimes needed)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

# yaml-cpp
find_package(yaml-cpp REQUIRED)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${YAML_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE
        ${YAML_CPP_LIBRARIES})

# fmt
find_package(fmt REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
        fmt::fmt)

# opengl
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
        ${OPENGL_LIBRARIES})

# qt
find_package(Qt5 5.15 COMPONENTS
        Core
        Gui
        Widgets
        OpenGL
        Xml
        Svg
        REQUIRED
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        Qt5::OpenGL
        Qt5::Xml
        Qt5::Svg
)

# gst
find_package(PkgConfig REQUIRED)
pkg_search_module(gstreamer REQUIRED IMPORTED_TARGET gstreamer-1.0>=1.20)
pkg_search_module(gstreamer-app REQUIRED IMPORTED_TARGET gstreamer-app-1.0>=1.20)
pkg_search_module(gstreamer-video REQUIRED IMPORTED_TARGET gstreamer-video-1.0>=1.20)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${GSTREAMER_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE
        PkgConfig::gstreamer
        PkgConfig::gstreamer-app
        PkgConfig::gstreamer-video
)

# qt-gst (deprecated)
find_package(Qt5GStreamer QUIET)
if (NOT Qt5GStreamer_FOUND)
    message(SEND_ERROR "Qt5GStreamer not found, which is required for qwidget5videosink")
endif ()

# libtorch
if (DEFINED ENV{Torch_DIR})
    # before find_package, we need to append libtorch install dir to CMAKE_PREFIX_PATH
    set(Torch_DIR $ENV{Torch_DIR})
    string(REPLACE ":" ";" CMAKE_PREFIX_PATH "$ENV{CMAKE_PREFIX_PATH}")
    list(APPEND CMAKE_PREFIX_PATH $ENV{Torch_DIR})
endif ()
find_package(Torch 2.0.0 REQUIRED)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${TORCH_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE
        ${TORCH_LIBRARIES})

# opencv
if (EXISTS /usr/local/lib/cmake/opencv4)
    # default opencv path when installed from source
    find_package(OpenCV REQUIRED PATHS /usr/local/lib/cmake/opencv4 NO_DEFAULT_PATH
            COMPONENTS core imgproc dnn highgui)
else ()
    find_package(OpenCV REQUIRED
            COMPONENTS core imgproc dnn highgui)
endif ()

target_include_directories(${PROJECT_NAME} PRIVATE
        ${OpenCV_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE
        opencv_core
        opencv_imgproc
        opencv_dnn
        opencv_highgui
)

# pillow-resize
find_package(PillowResize QUIET)

if (PillowResize_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${PILLOWRESIZE_LIBS})
    add_compile_definitions(WITH_PILLOW_RESIZE)
endif ()
