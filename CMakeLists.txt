cmake_minimum_required(VERSION 3.12)

# Set policies
cmake_policy(SET CMP0042 NEW) # ENABLE CMP0042: MACOSX_RPATH is enabled by default.
cmake_policy(SET CMP0048 NEW) # ENABLE CMP0048: Enable PROJECT_VERSION_XXXX variables.
cmake_policy(SET CMP0054 NEW) # ENABLE CMP0054: Only interpret if() arguments as variables or keywords when unquoted.
cmake_policy(SET CMP0063 NEW) # ENABLE CMP0063: Honor visibility properties for all target types.
cmake_policy(SET CMP0077 NEW) # ENABLE CMP0077: option() honors normal variables

if(WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
    add_definitions(-DNOGDI)
    add_definitions(-DNOGDICAPMASKS)
    add_definitions(-DNOMINMAX)
	add_definitions(-D_WIN32_WINNT=0x0A00)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Include cmake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

# Include custom cmake modules
include(cmake/modules/GetGitRevisionDescription.cmake)
include(cmake/modules/Healthcheck.cmake)

# 
# Project description and (meta) information
# 

# Get git revision
get_git_head_revision(GIT_REFSPEC GIT_SHA1)
string(SUBSTRING "${GIT_SHA1}" 0 12 GIT_REV)
if(NOT GIT_SHA1)
    set(GIT_REV "0")
endif()

# Meta information about the project
set(META_PROJECT_NAME        "aqualinkd")
set(META_PROJECT_DESCRIPTION "")
set(META_AUTHOR_ORGANIZATION "")
set(META_AUTHOR_DOMAIN       "")
set(META_AUTHOR_MAINTAINER   "")
set(META_VERSION_MAJOR       "1")
set(META_VERSION_MINOR       "4")
set(META_VERSION_PATCH       "0")
set(META_VERSION_REVISION    "${GIT_REV}")
set(META_VERSION             "${META_VERSION_MAJOR}.${META_VERSION_MINOR}.${META_VERSION_PATCH}")
set(META_NAME_VERSION        "${META_PROJECT_NAME} v${META_VERSION} (${META_VERSION_REVISION})")
set(META_CMAKE_INIT_SHA      "${GIT_REV}")

string(MAKE_C_IDENTIFIER ${META_PROJECT_NAME} META_PROJECT_ID)
string(TOUPPER ${META_PROJECT_ID} META_PROJECT_ID)

# 
# Declare project
# 

# Generate folders for IDE targets (e.g., VisualStudio solutions)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(IDE_FOLDER "")

# Declare project
project(${META_PROJECT_NAME} C CXX)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

#
# Project Health Check Setup
#

# Configure health check tools
enable_cppcheck(ON)
enable_clang_tidy(ON)
#enable_coverage(ON)

# 
# Compiler settings and options
# 

include(cmake/compiler-options.cmake)
include(cmake/cross-platform.cmake)
include(cmake/tools/Tools.cmake)


# 
# Deployment/installation setup
# 

include(cmake/packaging/packaging.cmake)

# 
# Project modules
# 

add_subdirectory(src)
add_subdirectory(docs)
add_subdirectory(deploy)

# 
# Deployment (global project files)
# 

# Install version file
install(FILES "${PROJECT_BINARY_DIR}/VERSION" DESTINATION ${INSTALL_ROOT} COMPONENT runtime)

# Install cmake find script for the project
install(FILES ${META_PROJECT_NAME}-config.cmake DESTINATION ${INSTALL_ROOT} COMPONENT dev)

# Install the project meta files
install(FILES AUTHORS   DESTINATION ${INSTALL_ROOT} COMPONENT runtime)
install(FILES LICENSE   DESTINATION ${INSTALL_ROOT} COMPONENT runtime)
install(FILES README.md DESTINATION ${INSTALL_ROOT} COMPONENT runtime)

# Install runtime data
install(DIRECTORY ${PROJECT_SOURCE_DIR}/data DESTINATION ${INSTALL_DATA} COMPONENT runtime)
