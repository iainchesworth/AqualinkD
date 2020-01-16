cmake_minimum_required(VERSION 3.12)
project(aqualinkd_player C)

set(SOURCE_FILES 
  # NOTHING HERE
)

add_executable(aqualinkd_player ${SOURCE_FILES})

target_include_directories(aqualinkd_player 
  PUBLIC
    # NOTHING HERE
)

target_link_directories(aqualinkd_player 
  PUBLIC 
    ${AQUALINKD_INSTALL_LIB_DIR}
)

target_link_libraries(aqualinkd_player 
  LINK_PRIVATE 
    libaqualink
  
  LINK_PUBLIC 
	${LIBM_LIBRARIES}
    Threads::Threads
)
