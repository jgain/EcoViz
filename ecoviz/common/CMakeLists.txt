set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(COMMON_SOURCES
    initialize.cpp
    mathutils.cpp
    progress.cpp
    region.cpp
    stats.cpp
    timer2.cpp)

if (BUILD_SOURCE2CPP)
    set(KERNELS
        ../../resources/shaders/basic.vert
        ../../resources/shaders/basic.frag
        ../../resources/shaders/genNormal.vert
        ../../resources/shaders/genNormal.frag
        ../../resources/shaders/simple.vert
        ../../resources/shaders/simple.frag
        ../../resources/shaders/phong.vert
        ../../resources/shaders/phong.frag
        ../../resources/shaders/phongRS.vert
        ../../resources/shaders/phongRS.frag
        ../../resources/shaders/phongRSmanip.vert
        ../../resources/shaders/phongRSmanip.frag
        ../../resources/shaders/rad_scaling_pass1.vert
        ../../resources/shaders/rad_scaling_pass1.frag
        ../../resources/shaders/rad_scaling_pass2.vert
        ../../resources/shaders/rad_scaling_pass2.frag
        ../../resources/shaders/sun.vert
        ../../resources/shaders/sun.frag
        ../../resources/shaders/canopy.vert
        ../../resources/shaders/canopy.frag)
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
