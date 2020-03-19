include(proto/CMakeLists.txt)
include(common/CMakeLists.txt)
include(op/CMakeLists.txt)
include(mempool/CMakeLists.txt)
include(framework/CMakeLists.txt)
include(tools/CMakeLists.txt)
include(src/CMakeLists.txt)

include_directories(SYSTEM  ${CMAKE_CURRENT_LIST_DIR}/../kvdb/include)

add_library(pdserving ${pdserving_srcs})
set_source_files_properties(
        ${pdserving_srcs}
        PROPERTIES
        COMPILE_FLAGS  "-Wno-strict-aliasing -Wno-unused-variable -Wno-non-virtual-dtor -Wno-error=non-virtual-dtor -Wno-error=delete-non-virtual-dtor")
add_dependencies(pdserving protobuf kvdb boost brpc leveldb pdcodegen configure)

target_link_libraries(pdserving
        brpc protobuf boost leveldb configure kvdb -lpthread -lcrypto -lm -lrt -lssl -ldl -lz)

# install
install(TARGETS pdserving
        RUNTIME DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/bin
        ARCHIVE DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/lib
        LIBRARY DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/so
        )

FILE(GLOB inc_common ${CMAKE_CURRENT_LIST_DIR}/common/*.h)
install(FILES ${inc_common}
        DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/include/core/predictor/common)

FILE(GLOB inc_framework ${CMAKE_CURRENT_LIST_DIR}/framework/*.h)
install(FILES ${inc_framework}
        DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/include/core/predictor/framework)

install(FILES ${CMAKE_CURRENT_LIST_DIR}/mempool/mempool.h
        DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/include/core/predictor/mempool)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/op/op.h
        DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/include/core/predictor/op)
