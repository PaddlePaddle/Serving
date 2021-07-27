
include(src/CMakeLists.txt)
include(proto/CMakeLists.txt)
add_library(sdk-cpp ${sdk_cpp_srcs})
add_dependencies(sdk-cpp pdcodegen configure protobuf brpc leveldb)
target_link_libraries(sdk-cpp brpc configure protobuf leveldb)

# install
install(TARGETS sdk-cpp
        ARCHIVE DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/lib
        )
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include
        DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/include/core/sdk-cpp/)
 
FILE(GLOB inc ${CMAKE_CURRENT_BINARY_DIR}/*.pb.h)
install(FILES ${inc}
        DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/include/core/sdk-cpp)
