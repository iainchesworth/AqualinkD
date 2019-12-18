cmake_minimum_required(VERSION 3.14)
project(libaqualinkd C)

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

add_library(libaqualinkd SHARED STATIC ${SOURCE_FILES})

target_include_directories(libaqualinkd
  PRIVATE
    ${AQUALINKD_HEADERS_DIR}

  PUBLIC
    # NOTHING HERE
)

install(TARGETS libaqualinkd DESTINATION ${AQUALINKD_INSTALL_LIB_DIR})
install(FILES aqualinkd.h DESTINATION ${AQUALINKD_INSTALL_INCLUDE_DIR})
