LIST(APPEND protofiles
	${CMAKE_CURRENT_LIST_DIR}/load_general_model_service.proto
	${CMAKE_CURRENT_LIST_DIR}/general_model_service.proto
)

PROTOBUF_GENERATE_SERVING_CPP(TRUE PROTO_SRCS PROTO_HDRS ${protofiles})
LIST(APPEND serving_srcs ${PROTO_SRCS})
