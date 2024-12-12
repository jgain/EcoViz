# This is needed for automoc
project(EcoViz)
cmake_minimum_required(VERSION 3.5)
include(CheckLibraryExists)
#set(CMAKE_PREFIX_PATH $ENV{HOME}/Qt/5.7/gcc_64)
#set(CMAKE_PREFIX_PATH $ENV{HOME}/Qt/6.7/gcc_64)
#set(CMAKE_PREFIX_PATH $ENV{HOME}/Qt/6.7.2/gcc_64/)
set(CMAKE_PREFIX_PATH $ENV{HOME}/Qt/6.8.0/gcc_64/)
if (APPLE)
    set(CMAKE_PREFIX_PATH "/opt/homebrew/opt/qt@6/lib/cmake/Qt6")
    set(Qt6Widgets_DIR $/opt/homebrew/opt/qt@6/lib/cmake/Qt6Widgets)
endif()

option(SMOKE_TEST "run a smoke test on every build" 1)
option(COVERAGE "compile for use with gcov" 0)
option(ASAN "compile with the address sanitiser" 0)
option(TSAN "compile with the thread sanitiser" 0)
option(SYNTHESIS_STATS "collect extra statistics about synthesis" 0)
enable_testing()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

### Check for dependencies
set(Boost_USE_MULTITHREADED ON)
add_definitions(-DBOOST_FILESYSTEM_VERSION=3)
find_package(Boost REQUIRED COMPONENTS filesystem program_options serialization system)
find_package(PkgConfig REQUIRED)
find_package(PythonInterp)
find_package(Qt6 COMPONENTS Core Gui Charts REQUIRED)
find_package(Qt6Widgets 6.7 REQUIRED)
find_package(Qt6OpenGL 6.7 REQUIRED)
cmake_policy(SET CMP0072 NEW)
find_package(OpenGL)
pkg_check_modules(GLEW glew)

### Installation specific. Will likely need to change these.
#set(Boost_INCLUDE_DIRS "/usr/local/Cellar/boost/1.67.0_1/include")
#set(Boost_LIB_DIRS "/usr/local/Cellar/boost/1.67.0_1/lib")
#set(GLEW_INCLUDE_DIRS "/usr/local/Cellar/glew/2.1.0/include/GL")

### Set include and link directories and definitions
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/khronos_headers
    ${Boost_INCLUDE_DIRS}
)
link_directories(
    ${PROJECT_SOURCE_DIR}/common
    ${PROJECT_SOURCE_DIR}/viz
    ${Boost_LIB_DIRS}
)

if (Qt6Widgets_FOUND)
    include_directories(${Qt6Widgets_INCLUDE_DIRS})
    add_definitions(${Qt6Widgets_DEFINITIONS})
endif()
if (GLEW_FOUND)
    include_directories(${GLEW_INCLUDE_DIRS})
    link_directories(${GLEW_LIBRARY_DIRS})
endif()
if (Qt6OpenGL_FOUND)
    include_directories(${Qt6OpenGL_INCLUDE_DIRS})
    add_definitions(${Qt6OpenGL_DEFINITIONS})
endif()
if (OPENGL_FOUND)
    include_directories(${OPENGL_INCLUDE_DIR})
endif()

### Check which components to build
if (PYTHONINTERP_FOUND AND OPENGL_FOUND)
    set(BUILD_SOURCE2CPP TRUE)
endif()
if (Qt6Widgets_FOUND
        AND Qt6OpenGL_FOUND
        AND OPENGL_FOUND
        AND GLEW_FOUND
        AND BUILD_SYNTHESIZER)
    set(BUILD_GUI TRUE)
endif()

### Set flags specific to the compiler
if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-enum-compare")
    if (APPLE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
    endif()
    if (NOT APPLE)
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_CXX_LINKER_FLAGS_RELEASE} -O4 -s")
    endif()
    # Link time optimization gives much better performance
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -flto")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb -D_GLIBCXX_DEBUG")
    set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} -flto")
endif()
if (CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-local-typedefs")
    if (${COVERAGE})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    endif()
    if (${ASAN})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libasan")
    endif()
    if (${TSAN})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -fpie -fno-omit-frame-pointer")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie -static-libtsan")
    endif()
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_definitions(-Dregister=)
endif()
if (SYNTHESIS_STATS)
    add_definitions(-DSYNTHESIS_STATS)
endif()

add_subdirectory(common)
add_subdirectory(viz)
add_subdirectory(tools)
