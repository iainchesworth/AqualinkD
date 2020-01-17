# Target name
set(target aqualink)

set(SOURCE_FILES
  config/config.c
  config/config_helpers.c
  config/config_parsers.c
  config/config_private.c
  config/config_private_defaults.c
  config/config_validators.c
  cross-platform/linux/daemon_linux.c
  cross-platform/linux/process_linux.c
  cross-platform/linux/serial_linux.c
  cross-platform/linux/signals_linux.c
  cross-platform/linux/socket_linux.c
  cross-platform/linux/terminal_linux.c
  cross-platform/linux/threads_linux.c
  cross-platform/linux/time_linux.c
  cross-platform/win32/daemon_win32.c
  cross-platform/win32/process_win32.c
  cross-platform/win32/serial_win32.c
  cross-platform/win32/signals_win32.c
  cross-platform/win32/socket_win32.c
  cross-platform/win32/terminal_win32.c
  cross-platform/win32/threads_win32.c
  cross-platform/win32/time_win32.c
  hardware/buttons/rs_buttons.c
  hardware/controllers/rs_controller.c
  hardware/devices/generic/generic_device.c
  hardware/devices/pda-remote/pda_remote.c
  hardware/devices/hardware_device.c
  hardware/devices/hardware_device_registry.c
  hardware/devices/hardware_device_registry_helpers.c
  hardware/devices/hardware_device_registry_private.c
  hardware/devices/hardware_device_types.c
  hardware/devices/hardware_device_utils.c
  hardware/heaters/heaters.c
  hardware/simulators/onetouch/onetouch_simulator_msg_long_handler.c
  hardware/simulators/onetouch/onetouch_simulator_private.c
  hardware/simulators/onetouch/onetouch_simulator_probe_handler.c
  hardware/simulators/onetouch/onetouch_simulator_status_handler.c
  hardware/simulators/onetouch/onetouch_simulator_types.c
  hardware/simulators/onetouch_simulator.c
  hardware/simulators/pda/pda_simulator_msg_long_handler.c
  hardware/simulators/pda/pda_simulator_private.c
  hardware/simulators/pda/pda_simulator_probe_handler.c
  hardware/simulators/pda/pda_simulator_status_handler.c
  hardware/simulators/pda/pda_simulator_types.c
  hardware/simulators/pda_simulator.c
  hardware/simulators/rs_keypadsimulator.c
  hardware/simulators/simulator_private.c
  hardware/simulators/simulator_types.c
  json/json_messages.c
  json/json_serializer.c
  json/json_string_utils.c
  logging/aqualink_default_logger.c
  logging/logging.c
  logging/logging_error_handler.c
  logging/logging_formatter.c
  logging/logging_levels.c
  logging/logging_message.c
  logging/logging_sink.c
  logging/logging_sink_basic_file.c
  logging/logging_sink_console.c
  logging/logging_sink_console_terminal.c
  logging/logging_sink_registry.c
  logging/logging_utils.c
  messages/message-processors/aq_serial_message_ack_handler.c
  messages/message-processors/aq_serial_message_msg_long_handler.c
  messages/message-processors/aq_serial_message_probe_handler.c
  messages/message-processors/aq_serial_message_status_handler.c
  messages/message-processors/aq_serial_message_unknown_handler.c
  messages/message-serializers/aq_serial_message_ack_serializer.c
  messages/message-serializers/aq_serial_message_msg_long_serializer.c
  messages/message-serializers/aq_serial_message_probe_serializer.c
  messages/message-serializers/aq_serial_message_status_serializer.c
  messages/message-serializers/aq_serial_message_unknown_serializer.c
  messages/aq_serial_message_ack.c
  messages/aq_serial_message_msg_long.c
  messages/aq_serial_message_probe.c
  messages/aq_serial_message_status.c
  messages/aq_serial_message_unknown.c
  mqtt/mqtt_utils.c
  serial/aq_serial.c 
  serial/aq_serial_checksums.c 
  serial/aq_serial_data_logger.c
  serial/aq_serial_messages.c
  serial/aq_serial_reader.c
  serial/aq_serial_threaded.c
  serial/aq_serial_writer.c
  serial/aq_serial_writer_queue.c
  string/string_utils.c
  threads/thread_utils.c
  utility/utils.c 
  web/aq_web_authentication.c
  web/aq_web_connection_handler.c
  web/aq_web_error_methodnotallowed.c
  web/aq_web_error_notfound.c
  web/aq_web_page_controller.c
  web/aq_web_page_simple.c
  web/aq_web_threaded.c
  web/aq_web_websockets.c
)

# Create namespaced alias
add_library(${target} STATIC ${SOURCE_FILES})
add_library(${META_PROJECT_NAME}::${target} ALIAS ${target})

# Export library for downstream projects
export(TARGETS ${target} NAMESPACE ${META_PROJECT_NAME}:: FILE ${PROJECT_BINARY_DIR}/cmake/${target}/${target}-export.cmake)

# 
# Project options
# 

set_target_properties(${target}
    PROPERTIES
    ${DEFAULT_PROJECT_OPTIONS}
    FOLDER "${IDE_FOLDER}"
    VERSION ${META_VERSION}
    SOVERSION ${META_VERSION_MAJOR}
)

# 
# Include directories
# 

target_include_directories(${target}
    PRIVATE
    ${PROJECT_BINARY_DIR}/src/
    ${CMAKE_CURRENT_SOURCE_DIR}/
    ${CMAKE_CURRENT_BINARY_DIR}/

    PUBLIC
    ${DEFAULT_INCLUDE_DIRECTORIES}

    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/>
    $<INSTALL_INTERFACE:include>
)

# 
# Libraries
# 

find_package(Threads REQUIRED)
find_package(libwebsockets REQUIRED)

if(WIN32)

  find_package(unofficial-libconfuse CONFIG REQUIRED)
  target_link_libraries(${target} PUBLIC unofficial::libconfuse::libconfuse)
  
  find_package(json-c CONFIG REQUIRED)
  target_link_libraries(${target} PUBLIC json-c::json-c)
   
  # Add support for TinyCThread on Windows
  find_path(TINYCTHREAD_INCLUDE_DIR tinycthread.h)
  find_library(TINYCTHREAD_LIBRARY NAMES tinycthread)
  if (TINYCTHREAD_INCLUDE_DIR AND TINYCTHREAD_LIBRARY)
    target_include_directories(${target} SYSTEM PUBLIC ${TINYCTHREAD_INCLUDE_DIR})
    target_link_libraries(${target} PUBLIC ${TINYCTHREAD_LIBRARY})
  else()
    message(ERROR "Can not find the TinyCThread library for Windows.")
  endif()

  # Add support for the Win32 socket libraries
  find_library(WS2_32_LIBRARIES ws2_32)
  find_library(IPHLPAPI_LIBRARIES iphlpapi)
  if(WS2_32_LIBRARIES AND IPHLPAPI_LIBRARIES)
    target_link_libraries(${target} PUBLIC ${WS2_32_LIBRARIES})
    target_link_libraries(${target} PUBLIC ${IPHLPAPI_LIBRARIES})
  else()
    message(ERROR "Can not find socket libraries for Windows.")
  endif()

else()

  find_package(libconfuse REQUIRED)
  target_link_libraries(${target} PUBLIC ${LIBCONFUSE_LIBRARY})

  find_package(libjson-c REQUIRED) 
  target_link_libraries(${target} PUBLIC ${LIBJSON_C_LIBRARIES})
  
  find_package(libm REQUIRED)
  target_link_libraries(${target} PUBLIC ${LIBM_LIBRARIES})

endif()

target_link_libraries(${target}
    PRIVATE

    PUBLIC
    ${DEFAULT_LIBRARIES}
    Threads::Threads
    ${LIBWEBSOCKETS_LIBRARIES}

    INTERFACE
)

# 
# Compile definitions
# 

target_compile_definitions(${target}
    PRIVATE

    PUBLIC
    ${DEFAULT_COMPILE_DEFINITIONS}

    INTERFACE
)

# 
# Compile options
# 

target_compile_options(${target}
    PRIVATE

    PUBLIC
    ${DEFAULT_COMPILE_OPTIONS}

    INTERFACE
)


# 
# Linker options
# 

target_link_libraries(${target}
    PRIVATE

    PUBLIC
    ${DEFAULT_LINKER_OPTIONS}

    INTERFACE
)

# 
# Deployment
# 

# Library
install(TARGETS ${target}
    EXPORT  "${target}-export"            COMPONENT dev
    RUNTIME DESTINATION ${INSTALL_BIN}    COMPONENT runtime
    LIBRARY DESTINATION ${INSTALL_SHARED} COMPONENT runtime
    ARCHIVE DESTINATION ${INSTALL_LIB}    COMPONENT dev
)

# CMake config
install(EXPORT ${target}-export
    NAMESPACE   ${META_PROJECT_NAME}::
    DESTINATION ${INSTALL_CMAKE}/${target}
    COMPONENT   dev
)
