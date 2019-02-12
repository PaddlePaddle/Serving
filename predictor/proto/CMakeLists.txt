FILE(GLOB protofiles "${CMAKE_CURRENT_LIST_DIR}/*.proto")
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${protofiles})

add_library(proto STATIC ${PROTO_SRCS} ${PROTO_HDRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})
