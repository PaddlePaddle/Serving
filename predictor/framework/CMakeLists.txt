list(APPEND predictor_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/dag.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dag_view.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mc_cache.cpp
    ${CMAKE_CURRENT_LIST_DIR}/memory.cpp
    ${CMAKE_CURRENT_LIST_DIR}/op_repository.cpp
    ${CMAKE_CURRENT_LIST_DIR}/predictor_metric.cpp
    ${CMAKE_CURRENT_LIST_DIR}/resource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/server.cpp
    ${CMAKE_CURRENT_LIST_DIR}/service.cpp
    ${CMAKE_CURRENT_LIST_DIR}/workflow.cpp
    )
INCLUDE_DIRECTORIES(.)
