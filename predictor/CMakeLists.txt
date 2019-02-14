include(proto/CMakeLists.txt)
include(common/CMakeLists.txt)
include(op/CMakeLists.txt)
include(framework/CMakeLists.txt)
add_library(predictor ${predictor_srcs})
set_source_files_properties(
        ${predictor_srcs}
        PROPERTIES
        COMPILE_FLAGS  "-Wno-strict-aliasing -Wno-unused-variable -Wno-non-virtual-dtor -Wno-error=non-virtual-dtor -Wno-error=delete-non-virtual-dtor")
add_dependencies(predictor protobuf boost brpc)
target_include_directories(predictor PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/
        ${CMAKE_CURRENT_BINARY_DIR}/
        ${CMAKE_CURRENT_LIST_DIR}/../configure
        ${CMAKE_CURRENT_LIST_DIR}/../mempool
        ${CMAKE_CURRENT_LIST_DIR}/../spreg
        ${CMAKE_CURRENT_LIST_DIR}/../ullib/include
        ${CMAKE_CURRENT_BINARY_DIR}/../bsl/include)
