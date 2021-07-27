set(seq_gen_src ${CMAKE_CURRENT_LIST_DIR}/seq_generator.cpp  ${CMAKE_CURRENT_LIST_DIR}/seq_file.cpp ${CMAKE_CURRENT_LIST_DIR}/quant.cpp)
LIST(APPEND seq_gen_src ${PROTO_SRCS})
add_executable(seq_generator ${seq_gen_src})
target_link_libraries(seq_generator protobuf -lpthread)
