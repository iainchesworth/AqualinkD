cmake_minimum_required(VERSION 3.12)
project(log_reader C)

find_package(Threads REQUIRED)

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
    aqualink
  
  LINK_PUBLIC 
	${LIBM_LIBRARIES}
    Threads::Threads
)

