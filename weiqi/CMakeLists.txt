find_package(FFmpeg REQUIRED COMPONENTS avcodec avutil avformat)

project(weiqi-obs)

if(NOT Qt5Widgets_FOUND)
  message(STATUS "Qt5 not found - disable weiqi application.")
  return()
endif()


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/libobs")
add_definitions(${Qt5Widgets_DEFINITIONS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})

set(weiqi-obs_src weiqi-obs.cpp weiqi-obs.h)

add_executable(weiqi-obs ${weiqi-obs_src})

target_link_libraries(weiqi-obs
	libobs
	Qt5::Widgets)


define_graphic_modules(weiqi-obs)
