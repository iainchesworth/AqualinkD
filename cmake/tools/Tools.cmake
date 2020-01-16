if(WIN32)

  set(PKG_CONFIG_EXECUTABLE "${CMAKE_SOURCE_DIR}/cmake/tools/win32/pkg-config.exe")

else()

  # Enable clang-tidy on non-Windows platforms
  set(CMAKE_C_CLANG_TIDY clang-tidy -checks=-*,readability-*,warnings-as-errors=*)

endif()
