cmake_minimum_required(VERSION 3.12)
project(aqualink C)

set(SOURCE_FILES
  aq_programmer.c 
  aq_serial.c 
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

  PUBLIC
    # NOTHING HERE
)

install(TARGETS aqualink DESTINATION ${AQUALINKD_INSTALL_LIB_DIR})
install(FILES aqualinkd.h DESTINATION ${AQUALINKD_INSTALL_INCLUDE_DIR})
