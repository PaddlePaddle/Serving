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
add_dependencies(pdserving protobuf boost brpc pdcodegen)

target_include_directories(pdserving PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/
        ${CMAKE_CURRENT_BINARY_DIR}/
        ${CMAKE_CURRENT_LIST_DIR}/../configure
        ${CMAKE_CURRENT_LIST_DIR}/../mempool
        ${CMAKE_CURRENT_LIST_DIR}/../spreg
        ${CMAKE_CURRENT_LIST_DIR}/../ullib/include
        ${CMAKE_CURRENT_BINARY_DIR}/../bsl/include)
target_link_libraries(pdserving -lpthread -lcrypto -lm -lrt -lssl -ldl -lz)


add_library(pdclient ${pdclient_srcs})
set_source_files_properties(
        ${pdclient_srcs}
        PROPERTIES
        COMPILE_FLAGS  "-Wno-strict-aliasing -Wno-unused-variable -Wno-non-virtual-dtor -Wno-error=non-virtual-dtor -Wno-error=delete-non-virtual-dtor")
add_dependencies(pdclient protobuf boost brpc pdcodegen)

target_include_directories(pdclient PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/
        ${CMAKE_CURRENT_BINARY_DIR}/
        ${CMAKE_CURRENT_LIST_DIR}/../configure
        ${CMAKE_CURRENT_LIST_DIR}/../mempool
        ${CMAKE_CURRENT_LIST_DIR}/../spreg
        ${CMAKE_CURRENT_LIST_DIR}/../ullib/include
        ${CMAKE_CURRENT_BINARY_DIR}/../bsl/include)
target_link_libraries(pdclient -lpthread -lcrypto -lm -lrt -lssl -ldl -lz)

