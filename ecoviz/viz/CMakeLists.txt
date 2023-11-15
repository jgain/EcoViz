set (BUILD_GUI 1)
if (BUILD_GUI)

    set(CMAKE_AUTOMOC TRUE)
    set(CMAKE_INCLUDE_CURRENT_DIR TRUE)  # MOC files generated in the binary dir
        set(DATA_IMPORT_DIR ${PROJECT_SOURCE_DIR}/../data_importer)
        set(BASE_ALL_DIR ${PROJECT_SOURCE_DIR}/..)

    set(GUI_SOURCES
       glwidget.cpp glwidget.h
       gltransect.cpp gltransect.h
       timer.cpp timer.h
       pft.cpp pft.h
       eco.cpp eco.h
       descriptor.cpp descriptor.h
       dice_roller.cpp dice_roller.h
       scene.cpp scene.h
       stroke.cpp stroke.h
       terrain.cpp terrain.h
       shape.cpp shape.h
       typemap.cpp typemap.h
       vecpnt.cpp vecpnt.h
       view.cpp view.h
       window.cpp window.h
       shaderProgram.cpp
       timewindow.cpp timewindow.h
       chartwindow.cpp chartwindow.h
       trenderer.cpp trenderer.h
       ${DATA_IMPORT_DIR}/data_importer.cpp ${DATA_IMPORT_DIR}/data_importer.h
       ${BASE_ALL_DIR}/common/basic_types.h
       cohortsampler.cpp cohortsampler.h
       cohortmaps.cpp cohortmaps.h
       progressbar_window.cpp progressbar_window.h
       export_dialog.cpp export_dialog.h
)

   #set(CMAKE_BUILD_TYPE Debug)
   #set(CMAKE_BUILD_TYPE Release)
   #set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/build-ecoviz-Qt_5_10_0_GCC_64bit-Debug)
   #set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/build-ecoviz-Qt_5_10_0_GCC_64bit-Default)
    add_library(vgui ${GUI_SOURCES})
    target_include_directories(vgui PRIVATE ${PROJECT_SOURCE_DIR} ${BASE_ALL_DIR})
    target_link_libraries(vgui common
        ${GLEW_LIBRARIES} ${Qt5Widgets_LIBRARIES} ${Qt5OpenGL_LIBRARIES} ${Qt5Charts_LIBRARIES}
        ${OPENGL_LIBRARIES} ${Boost_PROGRAM_OPTIONS_LIBRARY})

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")
    add_executable(ecoviz main.cpp)
    target_include_directories(ecoviz PRIVATE ${PROJECT_SOURCE_DIR} ${BASE_ALL_DIR})
    target_link_libraries(ecoviz vgui sqlite3)
    add_executable(analyse_cohortmap analyse_cohortmap_main.cpp)
    target_include_directories(analyse_cohortmap PRIVATE ${PROJECT_SOURCE_DIR} ${BASE_ALL_DIR})
    if(APPLE)
      target_link_libraries(analyse_cohortmap vgui sqlite3)
    else()
      target_link_libraries(analyse_cohortmap vgui sqlite3 stdc++fs)
    endif()
    
# stdc++fs) - causes issue with Clang compilation, looks to be unnecessary for gcc
    ADD_DEFINITIONS(-DSONOMA_DB_FILEPATH="${PROJECT_SOURCE_DIR}/../european.db")

endif()
