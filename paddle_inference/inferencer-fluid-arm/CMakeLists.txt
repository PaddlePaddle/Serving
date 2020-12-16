FILE(GLOB fluid_arm_engine_srcs ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
add_library(fluid_arm_engine ${fluid_arm_engine_srcs})
target_include_directories(fluid_arm_engine PUBLIC
        ${CMAKE_BINARY_DIR}/Paddle/fluid_install_dir/)
add_dependencies(fluid_arm_engine pdserving extern_paddle configure)
target_link_libraries(fluid_arm_engine pdserving paddle_fluid -lpthread -lcrypto -lm -lrt -lssl -ldl -lz)

install(TARGETS fluid_arm_engine 
        ARCHIVE DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/lib
        )
