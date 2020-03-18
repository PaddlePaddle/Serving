
include_directories(SYSTEM ${CMAKE_CURRENT_LIST_DIR}/include)

set(SRC_LIST ${CMAKE_CURRENT_LIST_DIR}/src/rockskvdb_impl.cpp
            ${CMAKE_CURRENT_LIST_DIR}/src/param_dict_mgr_impl.cpp
            ${CMAKE_CURRENT_LIST_DIR}/src/mock_param_dict_impl.cpp
            ${CMAKE_CURRENT_LIST_DIR}/src/paddle_rocksdb.cpp)
       
add_library(kvdb ${SRC_LIST})
add_dependencies(kvdb rocksdb)
target_include_directories(kvdb PUBLIC
${THIRD_PARTY_PATH}/install/rocksdb/include)
install(TARGETS kvdb ARCHIVE DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/lib/)


set(SRC_LIST2 ${CMAKE_CURRENT_LIST_DIR}/src/rockskvdb_impl.cpp
            ${CMAKE_CURRENT_LIST_DIR}/src/param_dict_mgr_impl.cpp
            ${CMAKE_CURRENT_LIST_DIR}/src/mock_param_dict_impl.cpp
            ${CMAKE_CURRENT_LIST_DIR}/src/paddle_rocksdb.cpp)

file(GLOB kvdb_headers "${CMAKE_CURRENT_LIST_DIR}/include/kvdb/*.h")
install(FILES ${kvdb_headers} DESTINATION
      ${PADDLE_SERVING_INSTALL_DIR}/include/kvdb/)
