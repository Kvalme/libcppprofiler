cmake_minimum_required (VERSION 3.8)

project(libcppprofiler CXX)

include_directories(src)

set(libcppprofiler_SOURCES src/cppprofiler.cpp)
set(libcppprofiler_HEADERS src/cppprofiler.h)

add_library(cppprofiler STATIC ${libcppprofiler_SOURCES} ${libcppprofiler_HEADERS})

set_target_properties(cppprofiler PROPERTIES OUTPUT_NAME "cppprofiler")
set_target_properties(cppprofiler PROPERTIES PREFIX "lib")

set_target_properties(cppprofiler PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(cppprofiler PUBLIC ${libcppprofiler_SOURCE_DIR}/src)
target_compile_features(cppprofiler PUBLIC cxx_std_14)

install(TARGETS cppprofiler LIBRARY DESTINATION lib ARCHIVE DESTINATION lib PUBLIC_HEADER DESTINATION include)

add_subdirectory(tools)
