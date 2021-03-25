FILE(GLOB paddle_inference_engine_srcs ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
add_library(paddle_inference_engine ${paddle_inference_engine_srcs})
target_include_directories(paddle_inference_engine PUBLIC
        ${CMAKE_BINARY_DIR}/Paddle/fluid_install_dir/)
add_dependencies(paddle_inference_engine pdserving extern_paddle configure)
target_link_libraries(paddle_inference_engine pdserving paddle_inference -lpthread -lcrypto -lm -lrt -lssl -ldl -lz)

install(TARGETS paddle_inference_engine 
        ARCHIVE DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/lib
        )
