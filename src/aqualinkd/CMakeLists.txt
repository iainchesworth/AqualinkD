cmake_minimum_required(VERSION 3.12)
project(aqualinkd C)

find_package(Threads REQUIRED)

set(SOURCE_FILES 
  aqualinkd.c
)

add_executable(aqualinkd ${SOURCE_FILES})

target_include_directories(aqualinkd 
  PUBLIC
    # Nothing here
)

target_link_directories(aqualinkd 
  PUBLIC 
    ${AQUALINKD_INSTALL_LIB_DIR}
)

target_link_libraries(aqualinkd 
  PRIVATE 
    aqualink
    ${LIBM_LIBRARIES}
	Threads::Threads
  
  PUBLIC 
    # Nothing here
)

install(TARGETS aqualinkd DESTINATION ${AQUALINKD_INSTALL_BIN_DIR})