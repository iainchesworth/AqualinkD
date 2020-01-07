cmake_minimum_required(VERSION 3.12)
project(aqualink C)

if(WIN32)

  find_package(json-c CONFIG REQUIRED)
  find_package(unofficial-libconfuse CONFIG REQUIRED)

else()

  find_package(libconfuse REQUIRED)
  find_package(libjson-c REQUIRED) 
  find_package(libm REQUIRED)

endif()

find_package(libmicrohttpd REQUIRED)
find_package(Threads REQUIRED)

set(SOURCE_FILES
  config/config.c
  config/config_helpers.c
  config/config_parsers.c
  config/config_private.c
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
  hardware/buttons/buttons.c
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
  mqtt/mqtt_utils.c
  serial/aq_serial.c 
  serial/aq_serial_checksums.c 
  serial/aq_serial_data_logger.c
  serial/aq_serial_message_ack.c
  serial/aq_serial_message_probe.c
  serial/aq_serial_message_status.c
  serial/aq_serial_message_unknown.c
  serial/aq_serial_messages.c
  serial/aq_serial_statemachine.c
  serial/aq_serial_threaded.c
  string/string_utils.c
  threads/thread_utils.c
  web/aq_web.c
  web/aq_web_authentication.c
  web/aq_web_connection_handler.c
  web/aq_web_error_methodnotallowed.c
  web/aq_web_error_notfound.c
  web/aq_web_page_controller.c
  web/aq_web_page_simple.c
  web/aq_web_websockets.c
  aq_programmer.c 
  aquapure.c
  net_services.c 
  pda.c 
  pda_aq_programmer.c 
  pda_menu.c 
  pentair_messages.c 
  utils.c 
)

if (CMAKE_BUILD_TYPE EQUAL "DEBUG")
    
target_compile_definitions(aqualink
  PUBLIC
    AQ_DEBUG
)

set(SOURCE_FILES
  ${SOURCES_FILES}
  timespec_subtract.c
)

endif (CMAKE_BUILD_TYPE EQUAL "DEBUG")

add_library(aqualink STATIC ${SOURCE_FILES})

target_compile_definitions(aqualink
  PUBLIC
    MG_DISABLE_MD5
    MG_DISABLE_HTTP_DIGEST_AUTH
    MG_DISABLE_MD5
    MG_DISABLE_JSON_RPC
)

target_include_directories(aqualink
  PRIVATE
    ${AQUALINKD_HEADERS_DIR}
    ${LIBJSON_C_INCLUDE_DIRS}
    ${LIBMICROHTTPD_INCLUDE_DIRS}

  PUBLIC
    ${LIBCONFUSE_INCLUDE_DIR}
)

if(WIN32)

target_link_libraries(aqualink
  PRIVATE 
	Threads::Threads
    json-c::json-c
    unofficial::libconfuse::libconfuse

  INTERFACE
    ${LIBMICROHTTPD_LIBRARIES}

  PUBLIC
    # Nothing here
)

else()

target_link_libraries(aqualink
  PRIVATE 
	Threads::Threads

  INTERFACE
    ${LIBCONFUSE_LIBRARY}
    ${LIBJSON_C_LIBRARIES}
    ${LIBM_LIBRARIES}
    ${LIBMICROHTTPD_LIBRARIES}

  PUBLIC
    # Nothing here
)

endif()
