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

add_executable(ximage ${CMAKE_CURRENT_LIST_DIR}/src/ximage.cpp)
target_link_libraries(ximage -Wl,--whole-archive sdk-cpp
               -Wl,--no-whole-archive -lpthread -lcrypto -lm -lrt -lssl -ldl
        -lz)

add_executable(ximage_press ${CMAKE_CURRENT_LIST_DIR}/src/ximage_press.cpp)
target_link_libraries(ximage_press -Wl,--whole-archive sdk-cpp
               -Wl,--no-whole-archive -lpthread -lcrypto -lm -lrt -lssl -ldl
        -lz)

add_executable(load_general_model ${CMAKE_CURRENT_LIST_DIR}/src/load_general_model.cpp)
target_link_libraries(load_general_model -Wl,--whole-archive sdk-cpp -Wl,--no-whole-archive
        -lpthread -lcrypto -lm -lrt -lssl -ldl
        -lz)

add_executable(echo ${CMAKE_CURRENT_LIST_DIR}/src/echo.cpp)
target_link_libraries(echo -Wl,--whole-archive sdk-cpp -Wl,--no-whole-archive
        -lpthread -lcrypto -lm -lrt -lssl -ldl
        -lz)

add_executable(echo_kvdb ${CMAKE_CURRENT_LIST_DIR}/src/echo_kvdb.cpp)
target_link_libraries(echo_kvdb -Wl,--whole-archive sdk-cpp -Wl,--no-whole-archive
        -lpthread -lcrypto -lm -lrt -lssl -ldl
         -lz)

add_executable(dense_format ${CMAKE_CURRENT_LIST_DIR}/src/dense_format.cpp)
target_link_libraries(dense_format -Wl,--whole-archive sdk-cpp -Wl,--no-whole-archive -lpthread -lcrypto -lm -lrt -lssl -ldl
        -lz)

add_executable(sparse_format ${CMAKE_CURRENT_LIST_DIR}/src/sparse_format.cpp)
target_link_libraries(sparse_format -Wl,--whole-archive sdk-cpp -Wl,--no-whole-archive -lpthread -lcrypto -lm -lrt -lssl -ldl
        -lz)

add_executable(int64tensor_format ${CMAKE_CURRENT_LIST_DIR}/src/int64tensor_format.cpp)
target_link_libraries(int64tensor_format -Wl,--whole-archive sdk-cpp -Wl,--no-whole-archive -lpthread -lcrypto -lm -lrt -lssl -ldl
        -lz)

add_executable(text_classification
        ${CMAKE_CURRENT_LIST_DIR}/src/text_classification.cpp)
target_link_libraries(text_classification -Wl,--whole-archive sdk-cpp -Wl,--no-whole-archive -lpthread -lcrypto -lm -lrt -lssl -ldl
        -lz)

add_executable(text_classification_press
        ${CMAKE_CURRENT_LIST_DIR}/src/text_classification_press.cpp)
target_link_libraries(text_classification_press -Wl,--whole-archive sdk-cpp -Wl,--no-whole-archive -lpthread -lcrypto -lm -lrt -lssl -ldl
        -lz)

add_executable(ctr_prediction
        ${CMAKE_CURRENT_LIST_DIR}/src/ctr_prediction.cpp)
target_link_libraries(ctr_prediction -Wl,--whole-archive sdk-cpp
        -Wl,--no-whole-archive -lpthread -lcrypto -lm -lrt -lssl -ldl -lz)

add_executable(bert
    ${CMAKE_CURRENT_LIST_DIR}/src/bert_service.cpp)
target_link_libraries(bert -Wl,--whole-archive sdk-cpp 
    -Wl,--no-whole-archive -lpthread -lcrypto -lm -lrt -lssl -ldl -lz)
    
# install
install(TARGETS ximage
        RUNTIME DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/client/image_classification/bin)
install(TARGETS ximage_press
        RUNTIME DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/client/image_classification/bin)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/conf DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/client/image_classification/)

install(TARGETS echo
        RUNTIME DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/demo/client/echo/bin)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/conf DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/client/echo/)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/python/echo.py
        DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/demo/client/echo/python)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/php/echo.php
        DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/demo/client/echo/php)

install(TARGETS echo_kvdb
        RUNTIME DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/demo/client/echo_kvdb/bin)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/conf DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/client/echo_kvdb/)

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
install(FILES ${CMAKE_CURRENT_LIST_DIR}/python/text_classification.py
        DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/client/text_classification/python)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/php/text_classification.php
        DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/client/text_classification/php)



install(TARGETS ctr_prediction
        RUNTIME DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/client/ctr_prediction/bin)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/conf DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/client/ctr_prediction/)

install(TARGETS bert
    RUNTIME DESTINATION
    ${PADDLE_SERVING_INSTALL_DIR}/demo/client/bert/bin)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/conf DESTINATION
    ${PADDLE_SERVING_INSTALL_DIR}/demo/client/bert/)
