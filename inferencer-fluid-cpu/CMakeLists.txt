FILE(GLOB fluid_cpu_engine_srcs ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
add_library(fluid_cpu_engine ${fluid_cpu_engine_srcs})
target_include_directories(fluid_cpu_engine PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
        ${PADDLE_INCLUDE_DIR}
        ${CMAKE_BINARY_DIR}/Paddle/fluid_install_dir/)
add_dependencies(fluid_cpu_engine pdserving paddle_fluid)
target_link_libraries(fluid_cpu_engine pdserving -liomp5 -lmklml_intel -lpthread -lcrypto -lm -lrt -lssl -ldl -lz)

