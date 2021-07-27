include(plugin/CMakeLists.txt)
include(src/CMakeLists.txt)

PROTOBUF_GENERATE_CPP(pdcodegen_proto_srcs pdcodegen_proto_hdrs
        ${CMAKE_SOURCE_DIR}/core/predictor/proto/pds_option.proto)

LIST(APPEND pdcodegen_srcs ${pdcodegen_proto_srcs})

add_executable(pdcodegen ${pdcodegen_srcs})
add_dependencies(pdcodegen boost protobuf)
target_link_libraries(pdcodegen protobuf ${PROTOBUF_PROTOC_LIBRARY})

# install
install(TARGETS pdcodegen
        RUNTIME DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/bin
        ARCHIVE DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/lib
        LIBRARY DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/so
        )
