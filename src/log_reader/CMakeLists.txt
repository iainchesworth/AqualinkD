cmake_minimum_required(VERSION 3.14)
project(log_reader C)

set(SOURCE_FILES 
  log_reader.c
)

add_executable(log_reader ${SOURCE_FILES})

target_include_directories(log_reader 
  PUBLIC
    # NOTHING HERE
)

target_link_directories(log_reader 
  PUBLIC 
    ${AQUALINKD_INSTALL_LIB_DIR}
)

target_link_libraries(log_reader 
  LINK_PRIVATE 
    libaqualinkd
  
  LINK_PUBLIC 
	${LIBM_LIBRARIES}
    Threads::Threads
)

install(TARGETS log_reader DESTINATION ${AQUALINKD_INSTALL_BIN_DIR})
