include(proto/CMakeLists.txt)

file(GLOB sdk_cpp_srcs ${CMAKE_SOURCE_DIR}/sdk-cpp/src/*.cpp)
list(APPEND elasticctr_srcs ${elastic_ctr_cpp_srcs})
list(APPEND elasticctr_srcs ${sdk_cpp_srcs})
list(APPEND elasticctr_srcs
        ${CMAKE_CURRENT_LIST_DIR}/api/elastic_ctr_api.cpp)

add_library(elasticctr ${elasticctr_srcs})
target_link_libraries(elasticctr brpc configure protobuf leveldb)

add_executable(elastic_ctr_demo ${CMAKE_CURRENT_LIST_DIR}/demo/demo.cpp)
target_link_libraries(elastic_ctr_demo elasticctr -lpthread -lcrypto -lm -lrt -lssl -ldl -lz)

# install
install(TARGETS elastic_ctr_demo
        RUNTIME DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/elastic_ctr/client/bin)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/demo/conf DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/elastic_ctr/client/)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/demo/data/ctr_prediction DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/elastic_ctr/client/data)
