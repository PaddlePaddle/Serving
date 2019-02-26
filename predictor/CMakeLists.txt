include(proto/CMakeLists.txt)
include(common/CMakeLists.txt)
include(op/CMakeLists.txt)
include(framework/CMakeLists.txt)
include(plugin/CMakeLists.txt)
include(src/CMakeLists.txt)

add_executable(pdcodegen ${pdcodegen_srcs})
target_link_libraries(pdcodegen protobuf ${PROTOBUF_PROTOC_LIBRARY})
target_include_directories(pdcodegen PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/
        ${CMAKE_CURRENT_BINARY_DIR}/)

add_library(pdserving ${pdserving_srcs})
set_source_files_properties(
        ${pdserving_srcs}
        PROPERTIES
        COMPILE_FLAGS  "-Wno-strict-aliasing -Wno-unused-variable -Wno-non-virtual-dtor -Wno-error=non-virtual-dtor -Wno-error=delete-non-virtual-dtor")
add_dependencies(pdserving protobuf boost brpc leveldb pdcodegen 
        configure ullib mempool)

target_include_directories(pdserving PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/
        ${CMAKE_CURRENT_BINARY_DIR}/
        ${CMAKE_CURRENT_BINARY_DIR}/../configure
        ${CMAKE_CURRENT_LIST_DIR}/../configure/include
        ${CMAKE_CURRENT_LIST_DIR}/../mempool
        ${CMAKE_CURRENT_LIST_DIR}/../ullib/include)
target_link_libraries(pdserving
        brpc protobuf boost leveldb configure ullib 
        mempool -lpthread -lcrypto -lm -lrt -lssl -ldl -lz)

add_executable(pdserving_exe ${pdserving_srcs})
set_source_files_properties(
        ${pdserving_srcs}
        PROPERTIES
        COMPILE_FLAGS  "-Wno-strict-aliasing -Wno-unused-variable -Wno-non-virtual-dtor -Wno-error=non-virtual-dtor -Wno-error=delete-non-virtual-dtor")
add_dependencies(pdserving_exe
        protobuf boost brpc leveldb pdcodegen configure 
        ullib mempool)

target_include_directories(pdserving_exe PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/
        ${CMAKE_CURRENT_BINARY_DIR}/
        ${CMAKE_CURRENT_BINARY_DIR}/../configure
        ${CMAKE_CURRENT_LIST_DIR}/../configure/include
        ${CMAKE_CURRENT_LIST_DIR}/../mempool
        ${CMAKE_CURRENT_LIST_DIR}/../ullib/include)
target_link_libraries(pdserving_exe brpc protobuf leveldb 
        configure ullib mempool -lpthread -lcrypto -lm -lrt -lssl
        -ldl -lz)

add_library(pdclient ${pdclient_srcs})
set_source_files_properties(
        ${pdclient_srcs}
        PROPERTIES
        COMPILE_FLAGS  "-Wno-strict-aliasing -Wno-unused-variable -Wno-non-virtual-dtor -Wno-error=non-virtual-dtor -Wno-error=delete-non-virtual-dtor")
add_dependencies(pdclient protobuf boost brpc pdcodegen configure)

target_include_directories(pdclient PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/
        ${CMAKE_CURRENT_BINARY_DIR}/
        ${CMAKE_CURRENT_BINARY_DIR}/../configure
        ${CMAKE_CURRENT_LIST_DIR}/../configure/include
        ${CMAKE_CURRENT_LIST_DIR}/../mempool
        ${CMAKE_CURRENT_LIST_DIR}/../ullib/include)
target_link_libraries(pdclient protobuf boost brpc -lpthread -lcrypto -lm -lrt -lssl -ldl -lz)

