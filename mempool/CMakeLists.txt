add_library(mempool ${CMAKE_CURRENT_LIST_DIR}/mempool.cpp)
add_dependencies(mempool brpc)
target_include_directories(mempool PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}/../bsl/include)
