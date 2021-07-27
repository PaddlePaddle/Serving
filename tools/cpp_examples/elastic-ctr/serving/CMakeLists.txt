if (NOT EXISTS
                ${CMAKE_CURRENT_LIST_DIR}/data/model/paddle/fluid/ctr_prediction)
        execute_process(COMMAND wget --no-check-certificate
                https://paddle-serving.bj.bcebos.com/data/ctr_prediction/elastic_ctr_model.tar.gz
                --output-document
                ${CMAKE_CURRENT_LIST_DIR}/data/model/paddle/fluid/elastic_ctr_model.tar.gz)
        execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf
                "${CMAKE_CURRENT_LIST_DIR}/data/model/paddle/fluid/elastic_ctr_model.tar.gz"
                WORKING_DIRECTORY
                ${CMAKE_CURRENT_LIST_DIR}/data/model/paddle/fluid)
        execute_process(COMMAND ${CMAKE_COMMAND} -E rename inference_only ctr_prediction
                WORKING_DIRECTORY
                ${CMAKE_CURRENT_LIST_DIR}/data/model/paddle/fluid)
endif()

include_directories(SYSTEM  ${CMAKE_CURRENT_LIST_DIR}/../kvdb/include)

include(op/CMakeLists.txt)
include(proto/CMakeLists.txt)
add_executable(elastic_serving ${serving_srcs})
add_dependencies(elastic_serving pdcodegen paddle_inference_engine pdserving paddle_inference cube-api)

target_include_directories(elastic_serving PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}/../../predictor
        )

target_link_libraries(elastic_serving -Wl,--whole-archive paddle_inference_engine
        -Wl,--no-whole-archive)

target_link_libraries(elastic_serving paddle_inference ${paddle_depend_libs})

target_link_libraries(elastic_serving pdserving)
target_link_libraries(elastic_serving cube-api)

target_link_libraries(elastic_serving kvdb rocksdb)

target_link_libraries(elastic_serving -liomp5 -lmklml_intel -lmkldnn -lpthread
        -lcrypto -lm -lrt -lssl -ldl -lz -lbz2)

install(TARGETS elastic_serving
        RUNTIME DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/elastic_ctr/serving/bin)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/conf DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/elastic_ctr/serving/)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/data DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/elastic_ctr/serving/)

FILE(GLOB inc ${CMAKE_CURRENT_BINARY_DIR}/*.pb.h)
install(FILES ${inc}
        DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/include/serving)

if (${WITH_MKL})
    install(FILES
            ${CMAKE_BINARY_DIR}/third_party/install/Paddle/third_party/install/mklml/lib/libmklml_intel.so
                ${CMAKE_BINARY_DIR}/third_party/install/Paddle/third_party/install/mklml/lib/libiomp5.so
                ${CMAKE_BINARY_DIR}/third_party/install/Paddle/third_party/install/mkldnn/lib/libmkldnn.so.1
                DESTINATION
                ${PADDLE_SERVING_INSTALL_DIR}/elastic_ctr/serving/bin)
endif()
