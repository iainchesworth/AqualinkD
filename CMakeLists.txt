cmake_minimum_required(VERSION 3.12)

if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNOMINMAX)
	add_definitions(-D_WIN32_WINNT=0x0A00)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

project(aqualinkd)

set(CMAKE_C_STANARD 11)
set(CMAKE_C_CLANG_TIDY clang-tidy -checks=-*,readability-*,warnings-as-errors=*)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,readability-*,warnings-as-errors=*;)

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})

set(AQUALINKD_INSTALL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(AQUALINKD_INSTALL_BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
set(AQUALINKD_INSTALL_LIB_DIR ${PROJECT_SOURCE_DIR}/lib)

set(AQUALINKD_HEADERS_DIR ${PROJECT_SOURCE_DIR}/src/libaqualink)

include_directories(${AQUALINKD_INSTALL_INCLUDE_DIR})
include_directories(${AQUALINKD_HEADERS_DIR})

add_subdirectory(src)
add_subdirectory(test)
