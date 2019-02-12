FILE(GLOB var_utils_srcs ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
add_library(var_utils ${var_utils_srcs})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
