if (WIN32)

  set(PKG_CONFIG_EXECUTABLE "${CMAKE_SOURCE_DIR}/cmake/tools/win32/pkg-config.exe")

endif (WIN32)

include(cmake/tools/profiling_perf.cmake)
include(cmake/tools/profiling_vtune.cmake)

include(cmake/tools/valgrind.cmake)
