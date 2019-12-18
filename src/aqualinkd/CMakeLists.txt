cmake_minimum_required(VERSION 3.14)
project(aqualinkd C)

set(SOURCE_FILES 
  aqualinkd.c
)

add_executable(aqualinkd ${SOURCE_FILES})

target_include_directories(aqualinkd 
  PUBLIC
    # NOTHING HERE
)

target_link_directories(aqualinkd 
  PUBLIC 
    ${AQUALINKD_INSTALL_LIB_DIR}
)

target_link_libraries(aqualinkd 
  LINK_PRIVATE 
    libaqualinkd
  
  LINK_PUBLIC 
    ${LIBM_LIBRARIES}
	Threads::Threads
)

install(TARGETS aqualinkd DESTINATION ${AQUALINKD_INSTALL_BIN_DIR})