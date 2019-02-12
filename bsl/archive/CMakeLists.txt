FILE(GLOB archive_srcs ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
add_library(archive ${archive_srcs})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
