set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(COMMON_SOURCES
    initialize.cpp
    mathutils.cpp
    progress.cpp
    region.cpp
    stats.cpp
    timer.cpp)

if (BUILD_SOURCE2CPP)
    set(KERNELS
        ../viz/shaders/basic.vert
        ../viz/shaders/basic.frag
        ../viz/shaders/genNormal.vert
        ../viz/shaders/genNormal.frag
        ../viz/shaders/simple.vert
        ../viz/shaders/simple.frag
        ../viz/shaders/phong.vert
        ../viz/shaders/phong.frag
        ../viz/shaders/phongRS.vert
        ../viz/shaders/phongRS.frag
        ../viz/shaders/phongRSmanip.vert
        ../viz/shaders/phongRSmanip.frag
        ../viz/shaders/rad_scaling_pass1.vert
        ../viz/shaders/rad_scaling_pass1.frag
        ../viz/shaders/rad_scaling_pass2.vert
        ../viz/shaders/rad_scaling_pass2.frag
        ../viz/shaders/sun.vert
        ../viz/shaders/sun.frag
        ../viz/shaders/canopy.vert
        ../viz/shaders/canopy.frag)
    add_custom_command(
        OUTPUT source2cpp.cpp
        COMMAND ${PYTHON_EXECUTABLE} source2cpp.py ${KERNELS} ${CMAKE_CURRENT_BINARY_DIR}/source2cpp.cpp
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        MAIN_DEPENDENCY source2cpp.py
        DEPENDS ${KERNELS}
        COMMENT "Creating C++ source for shaders and kernels" VERBATIM)
    list(APPEND COMMON_SOURCES source2cpp.cpp)
endif()

add_library(common ${COMMON_SOURCES})
target_link_libraries(common ${Boost_SERIALIZATION_LIBRARY})
