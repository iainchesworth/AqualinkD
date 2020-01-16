cmake_minimum_required(VERSION 3.12)
project(aqualinkd CXX)

find_package(Threads REQUIRED)

set(SOURCE_FILES 
  aqualinkd.c
  options.c
)

add_executable(aqualinkd ${SOURCE_FILES})

target_include_directories(aqualinkd 
  PRIVATE
    # Nothing here

  PUBLIC
    # Nothing here
)

target_link_libraries(aqualinkd
  PRIVATE 
    aqualink

  INTERFACE
    # Nothing here

  PUBLIC
    # Nothing here
)

install(TARGETS aqualinkd 
  RUNTIME DESTINATION "${PROJECT_NAME}"
)

include(${CMAKE_SOURCE_DIR}/cmake/tools/perfprofile/perfprofile.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/tools/valgrind/valgrind.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/packaging/packaging.cmake)
