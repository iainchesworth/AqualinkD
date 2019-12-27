cmake_minimum_required(VERSION 3.12)
project(aqualink C)

find_package(Threads REQUIRED)
find_package(libm REQUIRED)
find_package(libmicrohttpd REQUIRED)

set(SOURCE_FILES
  aq_programmer.c 
  serial/aq_serial.c 
  serial/aq_serial_message_ack.c
  serial/aq_serial_message_probe.c
  serial/aq_serial_message_status.c
  serial/aq_serial_message_unknown.c
  serial/aq_serial_messages.c
  serial/aq_serial_statemachine.c
  serial/aq_serial_threaded.c
  threads/thread_utils.c
  web/aq_web.c
  web/aq_web_authentication.c
  web/aq_web_connection_handler.c
  web/aq_web_error_methodnotallowed.c
  web/aq_web_error_notfound.c
  web/aq_web_page_controller.c
  web/aq_web_page_simple.c
  web/aq_web_websockets.c
  aquapure.c 
  config.c 
  init_buttons.c 
  json_messages.c 
  net_services.c 
  pda.c 
  pda_aq_programmer.c 
  pda_menu.c 
  packetLogger.c 
  pentair_messages.c 
  mongoose.c
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
    ${LIBMICROHTTPD_INCLUDE_DIRS}

  PUBLIC
    # Nothing here
)

target_link_libraries(aqualink
  PRIVATE 
	Threads::Threads

  INTERFACE
    ${LIBM_LIBRARIES}
    ${LIBMICROHTTPD_LIBRARIES}
  
  PUBLIC 
    # Nothing here
)

install(TARGETS aqualink DESTINATION ${AQUALINKD_INSTALL_LIB_DIR})
install(FILES aqualinkd.h DESTINATION ${AQUALINKD_INSTALL_INCLUDE_DIR})
