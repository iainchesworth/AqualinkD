cmake_minimum_required(VERSION 3.12)
project(aqualinkd)

configure_file("${CMAKE_SOURCE_DIR}/cmake/version/version.h.in" "${PROJECT_SOURCE_DIR}/libaqualink/version/version.h")

add_subdirectory(libaqualink)
add_subdirectory(aqualinkd)
add_subdirectory(serial_logger)
add_subdirectory(log_reader)
#add_subdirectory(aqualinkd_player)
