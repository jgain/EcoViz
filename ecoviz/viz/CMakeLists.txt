set (BUILD_GUI 1)
if (BUILD_GUI)

    set(CMAKE_AUTOMOC TRUE)
    set(CMAKE_INCLUDE_CURRENT_DIR TRUE)  # MOC files generated in the binary dir
        set(DATA_IMPORT_DIR ${PROJECT_SOURCE_DIR}/../data_importer)
        set(BASE_ALL_DIR ${PROJECT_SOURCE_DIR}/..)

    set(GUI_SOURCES
       glwidget.cpp glwidget.h
       gltransect.cpp gltransect.h
       gloverview.cpp gloverview.h
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

    add_library(vgui ${GUI_SOURCES})
    target_include_directories(vgui PRIVATE ${PROJECT_SOURCE_DIR} ${BASE_ALL_DIR})
    target_link_libraries(vgui common
        ${GLEW_LIBRARIES} ${Qt6Widgets_LIBRARIES} ${Qt6OpenGL_LIBRARIES} ${Qt6Charts_LIBRARIES}
        ${OPENGL_LIBRARIES} ${Boost_PROGRAM_OPTIONS_LIBRARY})

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt6Widgets_EXECUTABLE_COMPILE_FLAGS}")
    add_executable(ecoviz main.cpp)
    target_include_directories(ecoviz PRIVATE ${PROJECT_SOURCE_DIR} ${BASE_ALL_DIR})
    target_link_libraries(ecoviz vgui sqlite3)
    add_executable(analyse_cohortmap analyse_cohortmap_main.cpp)
    target_include_directories(analyse_cohortmap PRIVATE ${PROJECT_SOURCE_DIR} ${BASE_ALL_DIR})
    # target_link_libraries(analyse_cohortmap vgui sqlite3 stdc++fs)
    target_link_libraries(analyse_cohortmap vgui sqlite3)
  
    
# stdc++fs) - causes issue with Clang compilation, looks to be unnecessary for gcc
    ADD_DEFINITIONS(-DSONOMA_DB_FILEPATH="${PROJECT_SOURCE_DIR}/../european.db")

endif()
