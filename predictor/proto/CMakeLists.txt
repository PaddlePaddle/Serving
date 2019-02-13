FILE(GLOB protofiles "${CMAKE_CURRENT_LIST_DIR}/*.proto")
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${protofiles})
LIST(APPEND predictor_srcs ${PROTO_SRCS})
