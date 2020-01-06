cmake_minimum_required(VERSION 3.12)
project(aqualinkd_tests CXX)

enable_testing()
find_package(GTest REQUIRED)
find_package(libconfuse REQUIRED)
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

target_link_libraries(aqualinkd_tests 
  PRIVATE 
    aqualink
    GTest::GTest
    GTest::Main
    ${LIBCONFUSE_LIBRARY}

  PUBLIC 
    # Nothing here
)
