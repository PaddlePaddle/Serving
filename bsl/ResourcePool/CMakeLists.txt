FILE(GLOB ResourcePool_srcs ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
add_library(ResourcePool ${ResourcePool_srcs})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
