add_custom_target(copy_bsl_headers)
add_custom_command(TARGET copy_bsl_headers
        COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/include/bsl/ && cp ${CMAKE_CURRENT_LIST_DIR}/*.h ${CMAKE_CURRENT_BINARY_DIR}/include/bsl/
        COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/include/bsl/utils && cp ${CMAKE_CURRENT_LIST_DIR}/utils/*.h ${CMAKE_CURRENT_BINARY_DIR}/include/bsl/utils
        COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/include/bsl/alloc && cp ${CMAKE_CURRENT_LIST_DIR}/alloc/*.h ${CMAKE_CURRENT_BINARY_DIR}/include/bsl/alloc
        COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/include/bsl/archive && cp ${CMAKE_CURRENT_LIST_DIR}/archive/*.h ${CMAKE_CURRENT_BINARY_DIR}/include/bsl/archive
        # COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/include/bsl/containers && cp ${CMAKE_CURRENT_LIST_DIR}/containers/*.h ${CMAKE_CURRENT_BINARY_DIR}/include/bsl/containers
        COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/include/bsl/check_cast && cp ${CMAKE_CURRENT_LIST_DIR}/check_cast/*.h ${CMAKE_CURRENT_BINARY_DIR}/include/bsl/check_cast
        COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/include/bsl/ResourcePool && cp ${CMAKE_CURRENT_LIST_DIR}/ResourcePool/*.h ${CMAKE_CURRENT_BINARY_DIR}/include/bsl/ResourcePool
        COMMAND cp ${CMAKE_CURRENT_LIST_DIR}/check_cast/*.h ${CMAKE_CURRENT_BINARY_DIR}/include/bsl
        )
add_subdirectory(utils)
add_subdirectory(alloc)
add_subdirectory(archive)
add_subdirectory(containers)
add_subdirectory(pool)
add_subdirectory(buffer)
add_subdirectory(exception)
add_subdirectory(check_cast)
add_subdirectory(ResourcePool)
add_subdirectory(var/interface)
add_subdirectory(var/utils)
add_subdirectory(var/implement)
