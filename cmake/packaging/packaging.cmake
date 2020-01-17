# Check for system dir install
set(SYSTEM_DIR_INSTALL FALSE)
if("${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr" OR "${CMAKE_INSTALL_PREFIX}" STREQUAL "/usr/local")
    set(SYSTEM_DIR_INSTALL TRUE)
endif()

# Installation paths
if(UNIX AND SYSTEM_DIR_INSTALL)
    # Install into the system (/usr/bin or /usr/local/bin)
    set(INSTALL_ROOT      "share/${PROJECT_NAME}")       # /usr/[local]/share/<project>
    set(INSTALL_CMAKE     "share/${PROJECT_NAME}/cmake") # /usr/[local]/share/<project>/cmake
    set(INSTALL_EXAMPLES  "share/${PROJECT_NAME}")       # /usr/[local]/share/<project>
    set(INSTALL_DATA      "share/${PROJECT_NAME}")       # /usr/[local]/share/<project>
    set(INSTALL_BIN       "bin")                         # /usr/[local]/bin
    set(INSTALL_SHARED    "lib")                         # /usr/[local]/lib
    set(INSTALL_LIB       "lib")                         # /usr/[local]/lib
    set(INSTALL_INCLUDE   "include")                     # /usr/[local]/include
    set(INSTALL_DOC       "share/doc/${PROJECT_NAME}")   # /usr/[local]/share/doc/<project>
    set(INSTALL_SHORTCUTS "share/applications")          # /usr/[local]/share/applications
    set(INSTALL_ICONS     "share/pixmaps")               # /usr/[local]/share/pixmaps
    set(INSTALL_INIT      "/etc/init")                   # /etc/init (upstart/sysv/systemd init scripts)
else()
    # Install into local directory
    set(INSTALL_ROOT      ".")                           # ./
    set(INSTALL_CMAKE     "cmake")                       # ./cmake
    set(INSTALL_EXAMPLES  ".")                           # ./
    set(INSTALL_DATA      ".")                           # ./
    set(INSTALL_BIN       ".")                           # ./
    set(INSTALL_SHARED    "lib")                         # ./lib
    set(INSTALL_LIB       "lib")                         # ./lib
    set(INSTALL_INCLUDE   "include")                     # ./include
    set(INSTALL_DOC       "doc")                         # ./doc
    set(INSTALL_SHORTCUTS "misc")                        # ./misc
    set(INSTALL_ICONS     "misc")                        # ./misc
    set(INSTALL_INIT      "misc")                        # ./misc
endif()

# Set runtime path
set(CMAKE_SKIP_BUILD_RPATH            FALSE) # Add absolute path to all dependencies for BUILD
set(CMAKE_BUILD_WITH_INSTALL_RPATH    FALSE) # Use CMAKE_INSTALL_RPATH for INSTALL
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE) # Do NOT add path to dependencies for INSTALL

if(NOT SYSTEM_DIR_INSTALL)
    # Find libraries relative to binary
    if(APPLE)
        set(CMAKE_INSTALL_RPATH "@loader_path/../../../${INSTALL_LIB}")
    else()
        set(CMAKE_INSTALL_RPATH "$ORIGIN/${INSTALL_LIB}")       
    endif()
endif()
