#############################################################
# Find necessary libraries
#############################################################

find_package(Threads REQUIRED)
find_package(libmicrohttpd REQUIRED)
find_package(libwebsockets REQUIRED)

link_libraries(
  Threads::Threads
)

if(WIN32)

  find_package(json-c CONFIG REQUIRED)
  find_package(unofficial-libconfuse CONFIG REQUIRED)

  # Add support for GETOPT on Windows
  find_path(GETOPT_INCLUDE_DIR getopt.h)
  find_library(GETOPT_LIBRARY NAMES wingetopt getopt)
  if (GETOPT_INCLUDE_DIR AND GETOPT_LIBRARY)
    include_directories(SYSTEM ${GETOPT_INCLUDE_DIR})
    link_libraries(${GETOPT_LIBRARY})
  else()
    message(ERROR "Can not find getopt library for Windows.")
  endif()

  # Add support for TinyCThread on Windows
  find_path(TINYCTHREAD_INCLUDE_DIR tinycthread.h)
  find_library(TINYCTHREAD_LIBRARY NAMES tinycthread)
  if (TINYCTHREAD_INCLUDE_DIR AND TINYCTHREAD_LIBRARY)
    include_directories(SYSTEM ${TINYCTHREAD_INCLUDE_DIR})
    link_libraries(${TINYCTHREAD_LIBRARY})
  else()
    # Don't worry about errors for now..
  endif()

  # Add support for the VCPKG libmicrohttpd package
  find_file(LIBMICROHTTPD_LIBRARIES NAMES libmicrohttpd.lib)
  if(LIBMICROHTTPD_LIBRARIES)
    link_libraries(${LIBMICROHTTPD_LIBRARIES})
  else()
    message(ERROR "Can not find libmicrohttpd library for Windows.")
  endif()

  # Add support for the Win32 socket libraries
  find_library(WS2_32_LIBRARIES ws2_32)
  find_library(IPHLPAPI_LIBRARIES iphlpapi)
  if(WS2_32_LIBRARIES AND IPHLPAPI_LIBRARIES)
    link_libraries(${WS2_32_LIBRARIES})
    link_libraries(${IPHLPAPI_LIBRARIES})
  else()
    message(ERROR "Can not find socket libraries for Windows.")
  endif()

  
  link_libraries(
	${LIBMICROHTTPD_LIBRARIES}
    ${LIBWEBSOCKETS_LIBRARIES}
    json-c::json-c
    unofficial::libconfuse::libconfuse
  )
  
else(WIN32)

  find_package(libconfuse REQUIRED)
  find_package(libjson-c REQUIRED) 
  find_package(libm REQUIRED)
  
  link_libraries(
    ${LIBMICROHTTPD_LIBRARIES}
    ${LIBCONFUSE_LIBRARY}
    ${LIBJSON_C_LIBRARIES}
    ${LIBM_LIBRARIES}
    ${LIBWEBSOCKETS_LIBRARIES}
  )

endif(WIN32)

