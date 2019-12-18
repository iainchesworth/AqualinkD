cmake_minimum_required(VERSION 3.12)
project(aqualinkd)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
find_package(libm REQUIRED)

add_subdirectory(libaqualink)
add_subdirectory(aqualinkd)
add_subdirectory(serial_logger)
add_subdirectory(log_reader)
#add_subdirectory(aqualinkd_player)
