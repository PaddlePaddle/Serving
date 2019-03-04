include(proto/CMakeLists.txt)
include(common/CMakeLists.txt)
include(op/CMakeLists.txt)
include(mempool/CMakeLists.txt)
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
add_dependencies(pdserving protobuf boost brpc leveldb pdcodegen configure)

target_include_directories(pdserving PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/
        ${CMAKE_CURRENT_BINARY_DIR}/
        ${CMAKE_CURRENT_BINARY_DIR}/../configure
        ${CMAKE_CURRENT_LIST_DIR}/../configure/include
        )
target_link_libraries(pdserving
        brpc protobuf boost leveldb configure -lpthread -lcrypto -lm -lrt -lssl -ldl -lz)

# install
install(TARGETS pdserving pdcodegen
        RUNTIME DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/bin
        ARCHIVE DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/lib
        LIBRARY DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/so
        )
