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

find_library(MKLML_LIBS NAMES libmklml_intel.so libiomp5.so)
include(op/CMakeLists.txt)
include(proto/CMakeLists.txt)
add_executable(serving ${serving_srcs})
add_dependencies(serving pdcodegen fluid_cpu_engine pdserving paddle_fluid
        opencv_imgcodecs)
target_include_directories(serving PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}/../predictor
        )
target_link_libraries(serving opencv_imgcodecs
        ${opencv_depend_libs} -Wl,--whole-archive fluid_cpu_engine
        -Wl,--no-whole-archive pdserving paddle_fluid ${paddle_depend_libs}
        ${MKLML_LIB} ${MKLML_IOMP_LIB} -lpthread -lcrypto -lm -lrt -lssl -ldl -lz)

install(TARGETS serving
        RUNTIME DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/serving/bin)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/conf DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/serving/)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/data DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/serving/)
