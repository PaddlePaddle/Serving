
include_directories(SYSTEM ${CMAKE_CURRENT_LIST_DIR}/include)

set(SRC_LIST ${CMAKE_CURRENT_LIST_DIR}/src/test_rocksdb.cpp 
            ${CMAKE_CURRENT_LIST_DIR}/src/rockskvdb_impl.cpp
            ${CMAKE_CURRENT_LIST_DIR}/src/param_dict_mgr_impl.cpp
            ${CMAKE_CURRENT_LIST_DIR}/src/mock_param_dict_impl.cpp
            ${CMAKE_CURRENT_LIST_DIR}/src/paddle_rocksdb.cpp
            ${CMAKE_CURRENT_LIST_DIR}/src/gtest_kvdb.cpp)
       
add_library(kvdb ${SRC_LIST})
install(TARGETS kvdb ARCHIVE DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/lib/)

add_executable(kvdb_test ${SRC_LIST})
add_dependencies(kvdb_test rocksdb)
target_link_libraries(kvdb_test rocksdb bz2 snappy zlib gtest)
#target_include_directories(kvdb_test PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include/kvdb)

set(SRC_LIST2 ${CMAKE_CURRENT_LIST_DIR}/src/gtest_db_thread.cpp
            ${CMAKE_CURRENT_LIST_DIR}/src/rockskvdb_impl.cpp
            ${CMAKE_CURRENT_LIST_DIR}/src/param_dict_mgr_impl.cpp
            ${CMAKE_CURRENT_LIST_DIR}/src/mock_param_dict_impl.cpp
            ${CMAKE_CURRENT_LIST_DIR}/src/paddle_rocksdb.cpp)


add_executable(db_thread ${SRC_LIST2})
add_dependencies(db_thread rocksdb)
target_link_libraries(db_thread rocksdb bz2 snappy zlib gtest)
#target_include_directories(db_thread PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/include/kvdb)


set(SRC_LIST3 ${CMAKE_CURRENT_LIST_DIR}/src/gtest_db_func.cpp
             ${CMAKE_CURRENT_LIST_DIR}/src/rockskvdb_impl.cpp
             ${CMAKE_CURRENT_LIST_DIR}/src/param_dict_mgr_impl.cpp
             ${CMAKE_CURRENT_LIST_DIR}/src/mock_param_dict_impl.cpp
             ${CMAKE_CURRENT_LIST_DIR}/src/paddle_rocksdb.cpp)

add_executable(db_func ${SRC_LIST3})
add_dependencies(db_func rocksdb)
target_link_libraries(db_func rocksdb bz2 snappy zlib gtest)
#target_include_directories(db_func PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include/kvdb)

install(TARGETS kvdb_test 
        RUNTIME DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/demo/kvdb_test)
install(TARGETS db_thread
        RUNTIME DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/demo/db_thread)
install(TARGETS db_func
        RUNTIME DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/demo/db_func)


file(GLOB kvdb_headers "${CMAKE_CURRENT_LIST_DIR}/include/kvdb/*.h")
install(FILES ${kvdb_headers} DESTINATION
      ${PADDLE_SERVING_INSTALL_DIR}/include/kvdb/)
