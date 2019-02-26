LIST(APPEND protofiles
        ${CMAKE_CURRENT_LIST_DIR}/proto/server_configure.proto
        ${CMAKE_CURRENT_LIST_DIR}/proto/sdk_configure.proto
        ${CMAKE_CURRENT_LIST_DIR}/proto/inferencer_configure.proto
)

PROTOBUF_GENERATE_CPP(configure_proto_srcs configure_proto_hdrs ${protofiles})
list(APPEND proto_configure_srcs ${configure_proto_srcs})

list(APPEND proto_configure_srcs ${CMAKE_CURRENT_LIST_DIR}/src/configure_parser.cpp)

add_library(proto_configure ${proto_configure_srcs})

add_executable(test_configure
        ${CMAKE_CURRENT_LIST_DIR}/tests/test_configure.cpp)
target_include_directories(test_configure PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}/
        ${CMAKE_CURRENT_LIST_DIR}/include
        )
target_link_libraries(test_configure proto_configure protobuf)
