cmake_minimum_required(VERSION 3.12)
project(aqualinkd_tests CXX)

if(WIN32)

  find_package(json-c CONFIG REQUIRED)
  find_package(unofficial-libconfuse CONFIG REQUIRED)

else()

  find_package(libconfuse REQUIRED)
  find_package(libjson-c REQUIRED) 

endif()

find_package(Threads REQUIRED)

enable_testing()
find_package(GTest REQUIRED)
include(GoogleTest)

include_directories(${AQUALINKD_HEADERS_DIR})
include_directories(${GTEST_INCLUDE_DIRS})

set(SOURCE_FILES 
  src/aqualinkd_tests.cpp
  src/logging_tests.cpp
  src/json_tests.cpp
)

add_executable(aqualinkd_tests ${SOURCE_FILES})

gtest_discover_tests(aqualinkd_tests)

target_link_directories(aqualinkd 
  PUBLIC 
    ${AQUALINKD_INSTALL_LIB_DIR}
)

if(WIN32)

target_link_libraries(aqualinkd_tests
  PRIVATE 
	Threads::Threads
    GTest::GTest
    GTest::Main
    unofficial::libconfuse::libconfuse
)

else()

target_link_libraries(aqualinkd_tests
  PRIVATE 
	Threads::Threads
    GTest::GTest
    GTest::Main
    ${LIBCONFUSE_LIBRARY}
)

endif()
