FILE(GLOB common_srcs ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
LIST(APPEND pdserving_srcs ${common_srcs})
LIST(APPEND pdclient_srcs ${common_srcs})
