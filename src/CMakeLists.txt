cmake_minimum_required(VERSION 3.12)
project(aqualinkd)

configure_file("${CMAKE_SOURCE_DIR}/cmake/version/version.h.in" "${PROJECT_SOURCE_DIR}/libaqualink/version/version.h")

add_subdirectory(libaqualink)
add_subdirectory(aqualinkd)
add_subdirectory(serial_logger)
add_subdirectory(log_reader)
#add_subdirectory(aqualinkd_player)

#install(TARGETS aqualink DESTINATION ${AQUALINKD_INSTALL_LIB_DIR})
#install(FILES aqualinkd.h DESTINATION ${AQUALINKD_INSTALL_INCLUDE_DIR})

#install(TARGETS aqualinkd DESTINATION ${AQUALINKD_INSTALL_BIN_DIR})
#install(TARGETS aqualinkd_player DESTINATION ${AQUALINKD_INSTALL_BIN_DIR})
#install(TARGETS log_reader DESTINATION ${AQUALINKD_INSTALL_BIN_DIR})
#install(TARGETS serial_logger DESTINATION ${AQUALINKD_INSTALL_BIN_DIR})

if(WIN32)
    set(CPACK_GENERATOR ZIP)
    set(CPACK_SOURCE_GENERATOR ZIP)
else()
    set(CPACK_GENERATOR DEB)
    set(CPACK_SOURCE_GENERATOR DEB)
endif()

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(CPACK_PACKAGE_VERSION_TWEAK "${PROJECT_VERSION_TWEAK}")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Some maintainer goes here") #required
set(CPACK_PACKAGE_DESCRIPTION "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
set(CPACK_PACKAGE_HOMEPAGE_URL "${PROJECT_HOMEPAGE_URL}")
set(CPACK_PACKAGE_CHECKSUM SHA256)

include(CPack)