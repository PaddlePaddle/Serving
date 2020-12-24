FILE(GLOB fluid_gpu_engine_srcs ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
add_library(fluid_gpu_engine ${fluid_gpu_engine_srcs})
target_include_directories(fluid_gpu_engine PUBLIC
        ${CMAKE_BINARY_DIR}/Paddle/fluid_install_dir/)

add_dependencies(fluid_gpu_engine pdserving extern_paddle configure)
target_link_libraries(fluid_gpu_engine pdserving paddle_fluid iomp5 mklml_intel -lpthread -lcrypto -lm -lrt -lssl -ldl -lz)

install(TARGETS fluid_gpu_engine 
        ARCHIVE DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/lib
        )
