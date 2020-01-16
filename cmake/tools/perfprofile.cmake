if (MSVC) 
  message(STATUS "Enabling performance profiling options for MSVC")
  set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/PROFILE")
endif()
