
set(SOURCE_FILE cube-transfer.go)
add_go_executable(cube-transfer ${SOURCE_FILE})
add_dependencies(cube-transfer docopt-go)
add_dependencies(cube-transfer rfw)
add_dependencies(cube-transfer logex)
