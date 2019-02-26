include(src/CMakeLists.txt)
include(proto/CMakeLists.txt)
add_library(sdk-cpp ${sdk_cpp_srcs})
add_dependencies(sdk-cpp pdcodegen configure)
target_include_directories(sdk-cpp PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMKAE_CURRENT_BINARY_DIR}/
        ${CMAKE_CURRENT_BINARY_DIR}/../configure
        ${CMAKE_CURRENT_LIST_DIR}/../configure/include
        ${CMAKE_CURRENT_LIST_DIR}/../ullib/include
        )
target_link_libraries(sdk-cpp brpc configure protobuf leveldb)

add_executable(ximage ${CMAKE_CURRENT_LIST_DIR}/demo/ximage.cpp)
target_include_directories(ximage PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/../configure
        ${CMAKE_CURRENT_LIST_DIR}/../configure/include
        ${CMAKE_CURRENT_LIST_DIR}/../ullib/include)
target_link_libraries(ximage sdk-cpp -lpthread -lcrypto -lm -lrt -lssl -ldl
        -lz)

add_executable(mapcnn_dense ${CMAKE_CURRENT_LIST_DIR}/demo/mapcnn_dense.cpp)
target_include_directories(mapcnn_dense PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/
        ${CMAKE_CURRENT_BINARY_DIR}/../configure
        ${CMAKE_CURRENT_LIST_DIR}/../configure/include
        ${CMAKE_CURRENT_LIST_DIR}/../ullib/include)
target_link_libraries(mapcnn_dense sdk-cpp -lpthread -lcrypto -lm -lrt -lssl
        -ldl -lz)

add_executable(mapcnn_sparse ${CMAKE_CURRENT_LIST_DIR}/demo/mapcnn_sparse.cpp)
target_include_directories(mapcnn_sparse PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}/
        ${CMAKE_CURRENT_BINARY_DIR}/../configure
        ${CMAKE_CURRENT_LIST_DIR}/../configure/include
        ${CMAKE_CURRENT_LIST_DIR}/../ullib/include)
target_link_libraries(mapcnn_sparse sdk-cpp -lpthread -lcrypto -lm -lrt -lssl
        -ldl -lz)

