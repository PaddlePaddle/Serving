include(proto/CMakeLists.txt)

file(GLOB sdk_cpp_srcs ${CMAKE_SOURCE_DIR}/sdk-cpp/src/*.cpp)
list(APPEND elasticctr_srcs ${elastic_ctr_cpp_srcs})
list(APPEND elasticctr_srcs ${sdk_cpp_srcs})
list(APPEND elasticctr_srcs
        ${CMAKE_CURRENT_LIST_DIR}/api/elastic_ctr_api.cpp)

add_library(elasticctr SHARED ${elasticctr_srcs})
target_link_libraries(elasticctr brpc configure protobuf leveldb -lcrypto
        -lssl -lz -lrt)

set_target_properties(elasticctr PROPERTIES INTERFACE_LINK_LIBRARIES "")

add_executable(elastic_ctr_demo ${CMAKE_CURRENT_LIST_DIR}/demo/demo.cpp)
set_target_properties(elastic_ctr_demo PROPERTIES LINK_LIBRARIES "")
target_link_libraries(elastic_ctr_demo elasticctr -lpthread -lcrypto -lm -lrt
        -lssl -ldl -lz)

# install
install(TARGETS elasticctr elastic_ctr_demo
        RUNTIME DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/elastic_ctr/client/bin
        LIBRARY DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/elastic_ctr/client/bin)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/demo/conf DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/elastic_ctr/client/)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/demo/data/ctr_prediction DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/elastic_ctr/client/data)

install(TARGETS elasticctr
        LIBRARY DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/elastic_ctr/api/lib)

install(FILES ${CMAKE_CURRENT_LIST_DIR}/api/elastic_ctr_api.h
        DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/elastic_ctr/api/include/elastic-ctr/client/api/)
install(FILES
        ${CMAKE_BINARY_DIR}/elastic-ctr/client/elastic_ctr_prediction.pb.h
        ${CMAKE_BINARY_DIR}/elastic-ctr/client/pds_option.pb.h
        ${CMAKE_BINARY_DIR}/elastic-ctr/client/builtin_format.pb.h
        DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/elastic_ctr/api/include/elastic-ctr/client/)
install(DIRECTORY
        ${CMAKE_SOURCE_DIR}/sdk-cpp/include
        DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/elastic_ctr/api/include/sdk-cpp/)
install(DIRECTORY
        ${CMAKE_SOURCE_DIR}/configure/include
        DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/elastic_ctr/api/include/configure)
install(FILES
        ${CMAKE_BINARY_DIR}/configure/sdk_configure.pb.h
        DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/elastic_ctr/api/include/configure)

install(DIRECTORY
        ${CMAKE_BINARY_DIR}/third_party/install/protobuf/include/google
        ${CMAKE_BINARY_DIR}/third_party/install/brpc/include/brpc
        ${CMAKE_BINARY_DIR}/third_party/install/brpc/include/butil
        ${CMAKE_BINARY_DIR}/third_party/install/brpc/include/bthread
        ${CMAKE_BINARY_DIR}/third_party/install/brpc/include/bvar
        ${CMAKE_BINARY_DIR}/third_party/install/brpc/include/json2pb
        ${CMAKE_BINARY_DIR}/third_party/install/gflags/include/gflags
        ${CMAKE_BINARY_DIR}/third_party/install/glog/include/glog
        DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/elastic_ctr/api/include)

# Python client API
install(FILES ${CMAKE_CURRENT_LIST_DIR}/api/python/elasticctr/elastic_ctr_api.py
        DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/elastic_ctr/api/lib)

install(FILES ${CMAKE_CURRENT_LIST_DIR}/api/python/elasticctr/elastic_ctr_api.py
        DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/elastic_ctr/client/bin)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/demo/elastic_ctr.py
        DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/elastic_ctr/client/bin)
