include_directories(. ${libcppprofiler_SOURCE_DIR}/src)

add_executable(perftest perftest.cpp)
target_link_libraries(perftest cppprofiler-static)
