cmake_minimum_required(VERSION 3.14)
project(serial_logger C)

set(SOURCE_FILES 
  serial_logger.c
)

add_executable(serial_logger ${SOURCE_FILES})

target_include_directories(serial_logger 
  PUBLIC
    # NOTHING HERE
)

target_link_directories(serial_logger 
  PUBLIC 
    ${AQUALINKD_INSTALL_LIB_DIR}
)

target_link_libraries(serial_logger 
  LINK_PRIVATE 
    libaqualinkd
  
  LINK_PUBLIC 
	${LIBM_LIBRARIES}
    Threads::Threads
)

install(TARGETS serial_logger DESTINATION ${AQUALINKD_INSTALL_BIN_DIR})