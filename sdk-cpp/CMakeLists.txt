if (NOT EXISTS
                ${CMAKE_CURRENT_LIST_DIR}/data/text_classification/test_set.txt)
        execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory
                ${CMAKE_CURRENT_LIST_DIR}/data/text_classification)
        execute_process(COMMAND wget
                --no-check-certificate
                https://paddle-serving.bj.bcebos.com/data/text_classification/test_set.tar.gz
                --output-document
                ${CMAKE_CURRENT_LIST_DIR}/data/text_classification/test_set.tar.gz)

            execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf
                "${CMAKE_CURRENT_LIST_DIR}/data/text_classification/test_set.tar.gz"
                WORKING_DIRECTORY
                ${CMAKE_CURRENT_LIST_DIR}/data/text_classification
                )
endif()
include(src/CMakeLists.txt)
include(proto/CMakeLists.txt)
add_library(sdk-cpp ${sdk_cpp_srcs})
add_dependencies(sdk-cpp pdcodegen configure)
target_link_libraries(sdk-cpp brpc configure protobuf leveldb)
target_include_directories(sdk-cpp PUBLIC
        ${CMAKE_BINARY_DIR}/predictor/)

add_executable(ximage ${CMAKE_CURRENT_LIST_DIR}/demo/ximage.cpp)
target_link_libraries(ximage -Wl,--whole-archive sdk-cpp
               -Wl,--no-whole-archive -lpthread -lcrypto -lm -lrt -lssl -ldl
        -lz)

add_executable(echo ${CMAKE_CURRENT_LIST_DIR}/demo/echo.cpp)
target_link_libraries(echo -Wl,--whole-archive sdk-cpp -Wl,--no-whole-archive
        -lpthread -lcrypto -lm -lrt -lssl -ldl
        -lz)

add_executable(dense_format ${CMAKE_CURRENT_LIST_DIR}/demo/dense_format.cpp)
target_link_libraries(dense_format -Wl,--whole-archive sdk-cpp -Wl,--no-whole-archive -lpthread -lcrypto -lm -lrt -lssl -ldl
        -lz)

add_executable(sparse_format ${CMAKE_CURRENT_LIST_DIR}/demo/sparse_format.cpp)
target_link_libraries(sparse_format -Wl,--whole-archive sdk-cpp -Wl,--no-whole-archive -lpthread -lcrypto -lm -lrt -lssl -ldl
        -lz)

add_executable(int64tensor_format ${CMAKE_CURRENT_LIST_DIR}/demo/int64tensor_format.cpp)
target_link_libraries(int64tensor_format -Wl,--whole-archive sdk-cpp -Wl,--no-whole-archive -lpthread -lcrypto -lm -lrt -lssl -ldl
        -lz)

add_executable(text_classification
        ${CMAKE_CURRENT_LIST_DIR}/demo/text_classification.cpp)
target_link_libraries(text_classification -Wl,--whole-archive sdk-cpp -Wl,--no-whole-archive -lpthread -lcrypto -lm -lrt -lssl -ldl
        -lz)

add_executable(text_classification_press
        ${CMAKE_CURRENT_LIST_DIR}/demo/text_classification_press.cpp)
target_link_libraries(text_classification_press -Wl,--whole-archive sdk-cpp -Wl,--no-whole-archive -lpthread -lcrypto -lm -lrt -lssl -ldl
        -lz)

# install
install(TARGETS sdk-cpp
        ARCHIVE DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/lib
        )

install(TARGETS ximage
        RUNTIME DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/client/image_classification/bin)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/conf DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/client/image_classification/)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/data/images DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/client/image_classification/data)

install(TARGETS echo
        RUNTIME DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/demo/client/echo/bin)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/conf DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/client/echo/)

install(TARGETS dense_format
        RUNTIME DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/client/dense_format/bin)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/conf DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/client/dense_format/)

install(TARGETS sparse_format
        RUNTIME DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/client/sparse_format/bin)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/conf DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/client/sparse_format/)

install(TARGETS int64tensor_format
        RUNTIME DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/client/int64tensor_format/bin)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/conf DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/client/int64tensor_format/)

install(TARGETS text_classification text_classification_press
        RUNTIME DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/client/text_classification/bin)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/conf DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/client/text_classification/)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/data/text_classification DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/client/text_classification/data)

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include
        DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/)
