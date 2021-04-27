set (BUILD_GUI 1)
if (BUILD_GUI)

    set(CMAKE_AUTOMOC TRUE)
    set(CMAKE_INCLUDE_CURRENT_DIR TRUE)  # MOC files generated in the binary dir

	#set(BASE_DIR ${PROJECT_SOURCE_DIR}/../..)
        #set(BASE_DIR /home/konrad/EcoSynth)
        set(DATA_IMPORT_DIR ${PROJECT_SOURCE_DIR}/../data_importer)
        set(BASE_ALL_DIR ${PROJECT_SOURCE_DIR}/..)

    set(GUI_SOURCES
       glwidget.cpp
       timer.cpp
       pft.cpp
       eco.cpp
       grass.cpp
       descriptor.cpp
       dice_roller.cpp
       stroke.cpp
       terrain.cpp
       shape.cpp
       typemap.cpp
       vecpnt.cpp
       view.cpp
       window.cpp
       shaderProgram.cpp
       trenderer.cpp
       ${DATA_IMPORT_DIR}/data_importer.cpp
       ${BASE_ALL_DIR}/common/basic_types.h
       scrollwindow.cpp
       cohortsampler.cpp
       cohortmaps.cpp
       progressbar_window.cpp

)

   #set(CMAKE_BUILD_TYPE Debug)
   #set(CMAKE_BUILD_TYPE Release)
   #set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/build-ecoviz-Qt_5_10_0_GCC_64bit-Debug)
   #set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/build-ecoviz-Qt_5_10_0_GCC_64bit-Default)
    add_library(vgui ${GUI_SOURCES})
    target_include_directories(vgui PRIVATE ${PROJECT_SOURCE_DIR} ${BASE_ALL_DIR})
    target_link_libraries(vgui common
        ${GLEW_LIBRARIES} ${GLUT_LIBRARIES} ${Qt5Widgets_LIBRARIES} ${Qt5OpenGL_LIBRARIES}
        ${OPENGL_LIBRARIES} ${Boost_PROGRAM_OPTIONS_LIBRARY})

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")
    add_executable(ecoviz main.cpp)
    target_include_directories(ecoviz PRIVATE ${PROJECT_SOURCE_DIR} ${BASE_ALL_DIR})
    target_link_libraries(ecoviz vgui sqlite3)
    add_executable(analyse_cohortmap analyse_cohortmap_main.cpp)
    target_include_directories(analyse_cohortmap PRIVATE ${PROJECT_SOURCE_DIR} ${BASE_ALL_DIR})
    target_link_libraries(analyse_cohortmap vgui sqlite3 stdc++fs)
    add_executable(test_sampler ${PROJECT_SOURCE_DIR}/unittests/test_sampler.cpp)
    target_include_directories(test_sampler PRIVATE ${PROJECT_SOURCE_DIR} ${BASE_ALL_DIR})
    target_link_libraries(test_sampler vgui sqlite3 stdc++fs cppunit)

    ADD_DEFINITIONS(-DSONOMA_DB_FILEPATH="${PROJECT_SOURCE_DIR}/../sonoma.db")

endif()
