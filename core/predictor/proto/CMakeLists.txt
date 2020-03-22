PROTOBUF_GENERATE_CPP(pdcodegen_proto_srcs pdcodegen_proto_hdrs
        ${CMAKE_CURRENT_LIST_DIR}/pds_option.proto)

LIST(APPEND pdserving_srcs ${pdcodegen_proto_srcs})

LIST(APPEND protofiles
        ${CMAKE_CURRENT_LIST_DIR}/./builtin_format.proto
        ${CMAKE_CURRENT_LIST_DIR}/./msg_data.proto
        ${CMAKE_CURRENT_LIST_DIR}/./xrecord_format.proto
        ${CMAKE_CURRENT_LIST_DIR}/./framework.proto
)

PROTOBUF_GENERATE_SERVING_CPP(TRUE PROTO_SRCS PROTO_HDRS ${protofiles})
LIST(APPEND pdserving_srcs ${PROTO_SRCS} ${pdcodegen_proto_srcs})
