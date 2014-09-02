cmake_minimum_required (VERSION 2.6)

project(libcppprofiler CXX)

set(CMAKE_CXX_FLAGS "-std=c++11 -fPIC")

include_directories(src)

set(libcppprofiler_SOURCES src/cppprofiler.cpp)
set(libcppprofiler_HEADERS src/cppprofiler.h)

add_library(cppprofiler SHARED ${libcppprofiler_SOURCES})
add_library(cppprofiler-static STATIC ${libcppprofiler_SOURCES})

SET_TARGET_PROPERTIES(cppprofiler-static PROPERTIES OUTPUT_NAME "cppprofiler")
SET_TARGET_PROPERTIES(cppprofiler-static PROPERTIES PREFIX "lib")
SET_TARGET_PROPERTIES(cppprofiler PROPERTIES PUBLIC_HEADER ${libcppprofiler_HEADERS})

install( TARGETS cppprofiler cppprofiler-static LIBRARY DESTINATION lib ARCHIVE DESTINATION lib PUBLIC_HEADER DESTINATION include)

add_subdirectory(tools)
