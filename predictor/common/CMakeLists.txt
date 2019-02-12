add_library(common
    ${CMAKE_CURRENT_LIST_DIR}/constant.cpp
    )
add_dependencies(common brpc)
