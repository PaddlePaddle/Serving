FILE(GLOB spreg_srcs ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
add_library(spreg ${spreg_srcs})
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
message(STATUS ${CMAKE_CURRENT_LIST_DIR}/include)
