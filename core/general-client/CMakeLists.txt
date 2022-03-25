if(CLIENT)
add_subdirectory(pybind11)
pybind11_add_module(serving_client src/general_model.cpp src/pybind_general_model.cpp)
target_link_libraries(serving_client PRIVATE -Wl,--whole-archive utils sdk-cpp pybind python -Wl,--no-whole-archive -lpthread -lcrypto -lm -lrt -lssl -ldl -lz -Wl,-rpath,'$ORIGIN'/lib)
endif()

if(CLIENT)
FILE(GLOB client_srcs include/*.h src/client.cpp src/brpc_client.cpp)
add_library(client ${client_srcs})
add_dependencies(client utils sdk-cpp)
target_link_libraries(client utils sdk-cpp)
endif()

if(CLIENT)
include_directories(SYSTEM  ${CMAKE_CURRENT_LIST_DIR}/../../)
add_executable(simple_client example/simple_client.cpp)

add_dependencies(simple_client utils sdk-cpp client)

target_link_libraries(simple_client -Wl,--whole-archive
        -Wl,--no-whole-archive -Wl,--start-group -lpthread -lcrypto -lm -lrt -lssl -ldl -lz -Wl,-rpath,'$ORIGIN'/lib)

target_link_libraries(simple_client utils)
target_link_libraries(simple_client sdk-cpp)
target_link_libraries(simple_client client)
endif()