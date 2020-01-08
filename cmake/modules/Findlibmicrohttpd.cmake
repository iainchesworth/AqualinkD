if (NOT MSVC)
    include(FindPkgConfig)
    pkg_check_modules(PC_LIBMICROHTTPD "libmicrohttpd")
    if (NOT PC_LIBMICROHTTPD_FOUND)
        pkg_check_modules(PC_LIBMICROHTTPD "libmicrohttpd")
    endif (NOT PC_LIBMICROHTTPD_FOUND)
    if (PC_LIBMICROHTTPD_FOUND)
        # add CFLAGS from pkg-config file, e.g. draft api.
        add_definitions(${PC_LIBMICROHTTPD_CFLAGS} ${PC_LIBMICROHTTPD_CFLAGS_OTHER})
        # some libraries install the headers is a subdirectory of the include dir
        # returned by pkg-config, so use a wildcard match to improve chances of finding
        # headers and SOs.
        set(PC_LIBMICROHTTPD_INCLUDE_HINTS ${PC_LIBMICROHTTPD_INCLUDE_DIRS} ${PC_LIBMICROHTTPD_INCLUDE_DIRS}/*)
        set(PC_LIBMICROHTTPD_LIBRARY_HINTS ${PC_LIBMICROHTTPD_LIBRARY_DIRS} ${PC_LIBMICROHTTPD_LIBRARY_DIRS}/*)
    endif(PC_LIBMICROHTTPD_FOUND)
endif (NOT MSVC)

find_path (
    LIBMICROHTTPD_INCLUDE_DIRS
    NAMES microhttpd.h
    HINTS ${PC_LIBMICROHTTPD_INCLUDE_HINTS}
)
if (NOT MSVC)

find_library (
    LIBMICROHTTPD_LIBRARIES
    NAMES microhttpd
    HINTS ${PC_LIBMICROHTTPD_LIBRARY_HINTS}
)

else (NOT MSVC)

find_library (
    LIBMICROHTTPD_LIBRARIES
    NAMES libmicrohttpd
    HINTS ${PC_LIBMICROHTTPD_LIBRARY_HINTS}
)

endif (NOT MSVC)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
    LIBMICROHTTPD
    REQUIRED_VARS LIBMICROHTTPD_LIBRARIES LIBMICROHTTPD_INCLUDE_DIRS
)
mark_as_advanced(
    LIBMICROHTTPD_FOUND
    LIBMICROHTTPD_LIBRARIES LIBMICROHTTPD_INCLUDE_DIRS
)
