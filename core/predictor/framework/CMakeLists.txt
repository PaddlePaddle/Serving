FILE(GLOB framework_srcs ${CMAKE_CURRENT_LIST_DIR}/*.cpp ${CMAKE_CURRENT_LIST_DIR}/../../cube/cube-builder/src/seqfile_reader.cpp)
LIST(APPEND pdserving_srcs ${framework_srcs})
LIST(APPEND pclient_srcs ${framework_srcs})
