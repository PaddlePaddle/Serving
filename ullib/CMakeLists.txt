FILE(GLOB ullib_srcs ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
add_library(ullib ${ullib_srcs})
include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
message(STATUS ${CMAKE_CURRENT_LIST_DIR}/include)
