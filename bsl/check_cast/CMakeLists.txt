FILE(GLOB check_cast_srcs ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
add_library(check_cast ${check_cast_srcs})
add_dependencies(check_cast copy_bsl_headers)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
