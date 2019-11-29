LIST(APPEND protofiles
        ${CMAKE_CURRENT_LIST_DIR}/elastic_ctr_prediction.proto
)

PROTOBUF_GENERATE_SERVING_CPP(TRUE PROTO_SRCS PROTO_HDRS ${protofiles})
LIST(APPEND serving_srcs ${PROTO_SRCS})
