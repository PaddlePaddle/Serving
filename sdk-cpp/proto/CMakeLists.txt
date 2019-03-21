FILE(GLOB protos ${CMAKE_CURRENT_LIST_DIR}/*.proto)
list(APPEND protos ${CMAKE_SOURCE_DIR}/predictor/proto/pds_option.proto
        ${CMAKE_SOURCE_DIR}/predictor/proto/builtin_format.proto)
PROTOBUF_GENERATE_SERVING_CPP(FALSE PROTO_SRCS PROTO_HDRS ${protos})
LIST(APPEND sdk_cpp_srcs ${PROTO_SRCS})
