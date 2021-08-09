set(seq_gen_src ${CMAKE_CURRENT_LIST_DIR}/seq_generator.cpp  ${CMAKE_CURRENT_LIST_DIR}/seq_file.cpp ${CMAKE_CURRENT_LIST_DIR}/quant.cpp)
LIST(APPEND seq_gen_src ${PROTO_SRCS})
add_executable(seq_generator ${seq_gen_src})
target_link_libraries(seq_generator protobuf -lpthread)


set(seq_reader_src ${CMAKE_CURRENT_LIST_DIR}/seq_reader.cpp ${CMAKE_CURRENT_LIST_DIR}/../../cube/cube-builder/src/seqfile_reader.cpp)
add_executable(seq_reader ${seq_reader_src})
add_dependencies(seq_reader brpc)
install(TARGETS seq_reader
        RUNTIME DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/bin
        ARCHIVE DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/lib
        LIBRARY DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/so
        )

install(TARGETS seq_reader RUNTIME DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/tool)
install(TARGETS seq_generator RUNTIME DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/tool)
