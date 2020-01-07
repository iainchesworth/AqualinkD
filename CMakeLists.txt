cmake_minimum_required(VERSION 3.12)
cmake_policy(SET CMP0048 NEW) # Enables the PROJECT_VERSION_XXXX variables

if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNOGDI)
    add_definitions(-DNOGDICAPMASKS)
    add_definitions(-DNOMINMAX)
	add_definitions(-D_WIN32_WINNT=0x0A00)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

project(aqualinkd
  VERSION 1.4.0		  # Note this sets the CMAKE_PROJECT_VERSION_XXXXX variables as a by-product
  DESCRIPTION "A daemon to control Jandy Aqualink RS pool equipment from any home automation hub or web browser."
  HOMEPAGE_URL "https://github.com/sfeakes/AqualinkD/wiki"
)

include(${CMAKE_SOURCE_DIR}/cmake/tools/Tools.cmake)

set(CMAKE_C_STANDARD 11)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})

set(AQUALINKD_INSTALL_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(AQUALINKD_INSTALL_BIN_DIR ${PROJECT_SOURCE_DIR}/bin)
set(AQUALINKD_INSTALL_LIB_DIR ${PROJECT_SOURCE_DIR}/lib)

set(AQUALINKD_HEADERS_DIR ${PROJECT_SOURCE_DIR}/src/libaqualink)

include_directories(${AQUALINKD_INSTALL_INCLUDE_DIR})
include_directories(${AQUALINKD_HEADERS_DIR})

add_subdirectory(src)
add_subdirectory(test)
