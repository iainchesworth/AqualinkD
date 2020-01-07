cmake_minimum_required(VERSION 3.12)
project(aqualinkd C)

if(WIN32)
  find_package(unofficial-libconfuse CONFIG REQUIRED)
else()
  find_package(libconfuse REQUIRED)
endif()

find_package(Threads REQUIRED)

set(SOURCE_FILES 
  aqualinkd.c
  options.c
)

add_executable(aqualinkd ${SOURCE_FILES})

target_include_directories(aqualinkd 
  PUBLIC
    # Nothing here
)

target_link_directories(aqualinkd 
  PUBLIC 
    ${AQUALINKD_INSTALL_LIB_DIR}
)

if(WIN32)

target_link_libraries(aqualinkd
  PRIVATE 
    aqualink
	Threads::Threads
    unofficial::libconfuse::libconfuse

  INTERFACE
    # Nothing here

  PUBLIC
    # Nothing here
)

else()

target_link_libraries(aqualinkd
  PRIVATE 
    aqualink
	Threads::Threads

  INTERFACE
    ${LIBCONFUSE_LIBRARY}
    ${LIBM_LIBRARIES}

  PUBLIC
    # Nothing here
)

endif()

include(${CMAKE_SOURCE_DIR}/cmake/valgrind/valgrind.cmake)