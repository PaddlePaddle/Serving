FILE(GLOB pool_srcs ${CMAKE_CURRENT_LIST_DIR}/*.cpp) 
add_library(pool ${pool_srcs})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
