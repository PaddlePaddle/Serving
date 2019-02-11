list(APPEND predictor_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/common_echo_op.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dense_echo_op.cpp
    ${CMAKE_CURRENT_LIST_DIR}/op.cpp
    ${CMAKE_CURRENT_LIST_DIR}/sparse_echo_op.cpp
    ${CMAKE_CURRENT_LIST_DIR}/write_json_op.cpp
    )
INCLUDE_DIRECTORIES(.)
