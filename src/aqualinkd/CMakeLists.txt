# Target name
set(target aqualinkd)

set(SOURCE_FILES 
  aqualinkd.c
  options.c
)

# Create namespaced alias
add_executable(${target} ${SOURCE_FILES})
add_executable(${META_PROJECT_NAME}::${target} ALIAS ${target})

# 
# Project options
# 

set_target_properties(${target}
    PROPERTIES
    ${DEFAULT_PROJECT_OPTIONS}
    FOLDER "${IDE_FOLDER}"
)

# 
# Include directories
# 

target_include_directories(${target}
    PRIVATE
    ${DEFAULT_INCLUDE_DIRECTORIES}
    ${PROJECT_BINARY_DIR}
)

# 
# Libraries
# 

if(WIN32)

  # Add support for GETOPT on Windows
  find_path(GETOPT_INCLUDE_DIR getopt.h)
  find_library(GETOPT_LIBRARY NAMES wingetopt getopt)
  if (GETOPT_INCLUDE_DIR AND GETOPT_LIBRARY)
    include_directories(SYSTEM ${GETOPT_INCLUDE_DIR})
    target_link_libraries(${target} PRIVATE ${GETOPT_LIBRARY})
  else()
    message(ERROR "Can not find getopt library for Windows.")
  endif()

endif()

target_link_libraries(${target}
    PRIVATE
    ${DEFAULT_LIBRARIES}
    ${META_PROJECT_NAME}::aqualink
)

# 
# Compile definitions
# 

target_compile_definitions(${target}
    PRIVATE
    ${DEFAULT_COMPILE_DEFINITIONS}
)

# 
# Compile options
# 

target_compile_options(${target}
    PRIVATE
    ${DEFAULT_COMPILE_OPTIONS}
)

# 
# Linker options
# 

target_link_libraries(${target}
    PRIVATE
    ${DEFAULT_LINKER_OPTIONS}
)

# 
# Deployment
# 

# Executable
install(TARGETS ${target}
    RUNTIME DESTINATION ${INSTALL_BIN} COMPONENT examples
    BUNDLE  DESTINATION ${INSTALL_BIN} COMPONENT examples
)
