LIST(APPEND protofiles
        ${CMAKE_CURRENT_LIST_DIR}/image_class.proto
        ${CMAKE_CURRENT_LIST_DIR}/native_tensor.proto
)

PROTOBUF_GENERATE_SERVING_CPP(PROTO_SRCS PROTO_HDRS ${protofiles})
LIST(APPEND serving_srcs ${PROTO_SRCS} ${pdcodegen_proto_srcs})
