# This module tries to find libWebsockets library and include files
#
# LIBWEBSOCKETS_INCLUDE_DIRS, path where to find libwebsockets.h
# LIBWEBSOCKETS_LIBRARIES, the library to link against
# LIBWEBSOCKETS_FOUND, If false, do not try to use libWebSockets
#

if (NOT MSVC)
    include(FindPkgConfig)
    pkg_check_modules(PC_LIBWEBSOCKETS "libwebsockets")
    if (NOT PC_LIBWEBSOCKETS_FOUND)
        pkg_check_modules(PC_LIBWEBSOCKETS "libwebsockets")
    endif (NOT PC_LIBWEBSOCKETS_FOUND)
    if (PC_LIBWEBSOCKETS_FOUND)
        # add CFLAGS from pkg-config file, e.g. draft api.
        add_definitions(${PC_LIBWEBSOCKETS_CFLAGS} ${PC_LIBWEBSOCKETS_CFLAGS_OTHER})
        # some libraries install the headers is a subdirectory of the include dir
        # returned by pkg-config, so use a wildcard match to improve chances of finding
        # headers and SOs.
        set(PC_LIBWEBSOCKETS_INCLUDE_HINTS ${PC_LIBWEBSOCKETS_INCLUDE_DIRS} ${PC_LIBWEBSOCKETS_INCLUDE_DIRS}/*)
        set(PC_LIBWEBSOCKETS_LIBRARY_HINTS ${PC_LIBWEBSOCKETS_LIBRARY_DIRS} ${PC_LIBWEBSOCKETS_LIBRARY_DIRS}/*)
    endif(PC_LIBWEBSOCKETS_FOUND)
endif (NOT MSVC)

find_path (
    LIBWEBSOCKETS_INCLUDE_DIRS
    NAMES libwebsockets.h
    HINTS ${PC_LIBWEBSOCKETS_INCLUDE_HINTS}
)
if (NOT MSVC)

find_library (
    LIBWEBSOCKETS_LIBRARIES
    NAMES websockets
    HINTS ${PC_LIBWEBSOCKETS_LIBRARY_HINTS}
)

else (NOT MSVC)

find_library (
    LIBWEBSOCKETS_LIBRARIES
    NAMES WEBSOCKETS
    HINTS ${PC_LIBWEBSOCKETS_LIBRARY_HINTS}
)

endif (NOT MSVC)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
    LIBWEBSOCKETS
    REQUIRED_VARS LIBWEBSOCKETS_LIBRARIES LIBWEBSOCKETS_INCLUDE_DIRS
)
mark_as_advanced(
    LIBWEBSOCKETS_FOUND
    LIBWEBSOCKETS_LIBRARIES LIBWEBSOCKETS_INCLUDE_DIRS
)

