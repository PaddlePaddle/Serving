include_directories(SYSTEM ${CMAKE_CURRENT_LIST_DIR}/../../)
FILE(GLOB unittest_srcs ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
add_executable(unittest ${unittest_srcs})
set_source_files_properties(
        ${unittest_srcs}
        PROPERTIES
        COMPILE_FLAGS  "-Wextra -Wno-strict-aliasing -Wno-unused-variable -Wno-non-virtual-dtor -Wno-error=non-virtual-dtor -Wno-error=delete-non-virtual-dtor")
install(TARGETS unittest
        RUNTIME DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/unittest)
target_link_libraries(unittest pdserving gtest -lpthread)
