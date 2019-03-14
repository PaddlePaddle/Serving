LIST(APPEND protofiles
        ${CMAKE_CURRENT_LIST_DIR}/image_class.proto
        ${CMAKE_CURRENT_LIST_DIR}/dense_service.proto
        ${CMAKE_CURRENT_LIST_DIR}/sparse_service.proto
        ${CMAKE_CURRENT_LIST_DIR}/echo_service.proto
        ${CMAKE_CURRENT_LIST_DIR}/int64tensor_service.proto
        ${CMAKE_CURRENT_LIST_DIR}/text_classification.proto
)

PROTOBUF_GENERATE_SERVING_CPP(PROTO_SRCS PROTO_HDRS ${protofiles})
LIST(APPEND serving_srcs ${PROTO_SRCS})
