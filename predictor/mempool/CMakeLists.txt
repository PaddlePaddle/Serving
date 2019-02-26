FILE(GLOB mempool_srcs ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
LIST(APPEND pdserving_srcs ${mempool_srcs})
LIST(APPEND pdclient_srcs ${mempool_srcs})
