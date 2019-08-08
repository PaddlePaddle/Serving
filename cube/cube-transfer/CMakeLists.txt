set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

project(cube-transfer Go)

include(cmake/golang.cmake)

ExternalGoProject_Add(docopt-go github.com/docopt/docopt-go)
ExternalGoProject_Add(rfw github.com/mipearson/rfw)
ExternalGoProject_Add(logex github.com/Badangel/logex)

add_subdirectory(src)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/conf DESTINATION ${PADDLE_SERVING_INSTALL_DIR})
