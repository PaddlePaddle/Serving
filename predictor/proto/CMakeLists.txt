list(APPEND predictor_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/builtin_format.proto
    ${CMAKE_CURRENT_LIST_DIR}/dense_service.proto
    ${CMAKE_CURRENT_LIST_DIR}/echo_service.proto
    ${CMAKE_CURRENT_LIST_DIR}/image_classification.proto
    ${CMAKE_CURRENT_LIST_DIR}/int64tensor_service.proto
    ${CMAKE_CURRENT_LIST_DIR}/msg_data.proto
    ${CMAKE_CURRENT_LIST_DIR}/pds_option.proto
    ${CMAKE_CURRENT_LIST_DIR}/sparse_service.proto
    ${CMAKE_CURRENT_LIST_DIR}/xrecord_format.proto
    )
INCLUDE_DIRECTORIES(.)
