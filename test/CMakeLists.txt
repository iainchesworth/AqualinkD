cmake_minimum_required(VERSION 3.14)
project(aqualinkd_tests C)

find_package(Gtest REQUIRED)

include_directories(${AQUALINKD_HEADERS_DIR})
include_directories(${GTEST_INCLUDE_DIR})

set(SOURCE_FILES main.c src/aqualinkd_tests.c)

add_executable(aqualinkd_tests ${SOURCE_FILES})

add_test(test1 aqualinkd_tests COMMAND aqualinkd_tests)

target_link_libraries(aqualinkd_tests 
  LINK_PRIVATE 
    libaqualinkd 

  LINK_PUBLIC 
    optimized ${GTEST_LIBRARY}
	debug ${GTEST_LIBRARY_DEBUG}
)

install(TARGETS aqualinkd_tests DESTINATION ${AQUALINKD_INSTALL_BIN_DIR})
