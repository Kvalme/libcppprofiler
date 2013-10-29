include_directories(. ${libcppprofiler_SOURCE_DIR}/src)

add_executable(cppprof2xml cppprof2xml.cpp)
target_link_libraries(cppprof2xml cppprofiler-static)
