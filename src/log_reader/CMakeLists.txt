cmake_minimum_required(VERSION 3.12)
project(log_reader CXX)

set(SOURCE_FILES 
  log_reader.c
)

add_executable(log_reader ${SOURCE_FILES})

target_include_directories(log_reader 
  PRIVATE
    # Nothing here

  PUBLIC
    # Nothing here
)

target_link_libraries(log_reader
  PRIVATE 
    aqualink

  INTERFACE
    # Nothing here

  PUBLIC
    # Nothing here
)
