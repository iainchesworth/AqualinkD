cmake_minimum_required(VERSION 3.12)
project(serial_logger C)

set(SOURCE_FILES 
  serial-processor/serial_processor.c
  options.c
  serial_logger.c
)

add_executable(serial_logger ${SOURCE_FILES})

target_include_directories(serial_logger 
  PRIVATE
    # Nothing here

  PUBLIC
    # Nothing here
)

target_link_libraries(serial_logger
  PRIVATE 
    aqualink

  INTERFACE
    # Nothing here

  PUBLIC
    # Nothing here
)
