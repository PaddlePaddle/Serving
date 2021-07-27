if (NOT EXISTS
                ${CMAKE_CURRENT_LIST_DIR}/data/model/paddle/fluid/text_classification_lstm)
        execute_process(COMMAND wget
                --no-check-certificate https://paddle-serving.bj.bcebos.com/data/text_classification/text_classification_lstm.tar.gz
                --output-document
                ${CMAKE_CURRENT_LIST_DIR}/data/model/paddle/fluid/text_classification_lstm.tar.gz)

        execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf
                "${CMAKE_CURRENT_LIST_DIR}/data/model/paddle/fluid/text_classification_lstm.tar.gz"
                WORKING_DIRECTORY
                ${CMAKE_CURRENT_LIST_DIR}/data/model/paddle/fluid
                )
endif()

if (NOT EXISTS
                ${CMAKE_CURRENT_LIST_DIR}/data/model/paddle/fluid/ctr_prediction)
        execute_process(COMMAND wget --no-check-certificate
                https://paddle-serving.bj.bcebos.com/data/ctr_prediction/ctr_prediction.tar.gz
                --output-document
                ${CMAKE_CURRENT_LIST_DIR}/data/model/paddle/fluid/ctr_prediction.tar.gz)
        execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf
                "${CMAKE_CURRENT_LIST_DIR}/data/model/paddle/fluid/ctr_prediction.tar.gz"
                WORKING_DIRECTORY
                ${CMAKE_CURRENT_LIST_DIR}/data/model/paddle/fluid)
endif()

if (NOT EXISTS
                ${CMAKE_CURRENT_LIST_DIR}/data/model/paddle/fluid/bert_cased_L-12_H-768_A-12)
        execute_process(COMMAND wget --no-check-certificate
                https://paddle-serving.bj.bcebos.com/data/bert/bert_cased_L-12_H-768_A-12.tar.gz
                --output-document
                ${CMAKE_CURRENT_LIST_DIR}/data/model/paddle/fluid/bert_cased_L-12_H-768_A-12.tar.gz)
        execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf
            "${CMAKE_CURRENT_LIST_DIR}/data/model/paddle/fluid/bert_cased_L-12_H-768_A-12.tar.gz"
                WORKING_DIRECTORY
                ${CMAKE_CURRENT_LIST_DIR}/data/model/paddle/fluid)
endif()

include_directories(SYSTEM  ${CMAKE_CURRENT_LIST_DIR}/../kvdb/include)

include(op/CMakeLists.txt)
include(proto/CMakeLists.txt)
add_executable(serving ${serving_srcs})
add_dependencies(serving pdcodegen paddle_inference_engine pdserving paddle_inference
        opencv_imgcodecs cube-api)
if (WITH_GPU)
    add_dependencies(serving paddle_inference_engine)
endif()
target_include_directories(serving PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}/../../core/predictor
        )

if(WITH_GPU)
    target_link_libraries(serving -Wl,--whole-archive paddle_inference_engine
            -Wl,--no-whole-archive)
endif()

target_link_libraries(serving -Wl,--whole-archive paddle_inference_engine
        -Wl,--no-whole-archive)

target_link_libraries(serving paddle_inference ${paddle_depend_libs})

target_link_libraries(serving opencv_imgcodecs
        ${opencv_depend_libs})

target_link_libraries(serving pdserving)
target_link_libraries(serving cube-api)

target_link_libraries(serving kvdb rocksdb)

if(WITH_GPU)
    target_link_libraries(serving ${CUDA_LIBRARIES})
endif()

if(WITH_MKL)
    message("lalalala: " ${WITH_MKL})
    target_link_libraries(serving -liomp5 -lmklml_intel -lmkldnn -lpthread -lcrypto -lm -lrt -lssl -ldl -lz -lbz2)
else()
    message("hehehehe: " ${WITH_MKL})
    target_link_libraries(serving openblas -lpthread -lcrypto -lm -lrt -lssl -ldl -lz -lbz2)
endif()

install(TARGETS serving
        RUNTIME DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/serving/bin)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/conf DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/serving/)

FILE(GLOB inc ${CMAKE_CURRENT_BINARY_DIR}/*.pb.h)
install(FILES ${inc}
        DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/include/serving)

if (${WITH_MKL})
install(FILES
        ${CMAKE_BINARY_DIR}/third_party/install/Paddle/third_party/install/mklml/lib/libmklml_intel.so
        ${CMAKE_BINARY_DIR}/third_party/install/Paddle/third_party/install/mklml/lib/libiomp5.so
        ${CMAKE_BINARY_DIR}/third_party/install/Paddle/third_party/install/mkldnn/lib/libmkldnn.so.0
        DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/serving/bin)
endif()
