# 
# Configuration for all sub-projects
# 

# Generate version-header
configure_file(version.h.in ${CMAKE_CURRENT_BINARY_DIR}/${META_PROJECT_NAME}-version.h @ONLY)

# 
# Sub-projects
# 

set(IDE_FOLDER "")
add_subdirectory(libaqualink)
add_subdirectory(aqualinkd)
