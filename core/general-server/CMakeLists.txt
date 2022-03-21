include_directories(SYSTEM  ${CMAKE_CURRENT_LIST_DIR}/../../)
include(op/CMakeLists.txt)
include(proto/CMakeLists.txt)

add_executable(serving ${serving_srcs})
add_dependencies(serving pdcodegen paddle_inference_engine pdserving paddle_inference cube-api utils utf8proc)

if (WITH_GPU)
    add_dependencies(serving paddle_inference_engine)
endif()

if (WITH_LITE)
    add_dependencies(serving paddle_inference_engine)
endif()

target_include_directories(serving PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}/../../core/predictor
)
include_directories(${CUDNN_ROOT}/include/)

target_link_libraries(serving -Wl,--whole-archive paddle_inference_engine
        -Wl,--no-whole-archive)

if(WITH_OPENCV)
    target_link_libraries(serving ${OpenCV_LIBS})
endif()
target_link_libraries(serving paddle_inference ${paddle_depend_libs})
target_link_libraries(serving brpc)
target_link_libraries(serving protobuf)
target_link_libraries(serving pdserving)
target_link_libraries(serving cube-api)
target_link_libraries(serving utils)
target_link_libraries(serving utf8proc)
target_link_libraries(serving prometheus ${prometheus_libs})
if(WITH_ASCEND_CL AND NOT WITH_LITE)
    target_link_libraries(serving ascendcl acl_op_compiler)
endif()


if(WITH_GPU)
    target_link_libraries(serving ${CUDA_LIBRARIES})
endif()

if(WITH_MKL OR WITH_GPU)
    if (WITH_JETSON)
        target_link_libraries(serving openblas -lpthread -lcrypto -lm -lrt -lssl -ldl -lz -lbz2) 
    elseif (WITH_TRT)
    target_link_libraries(serving -liomp5 -lmklml_intel -lpthread -lcrypto -lm -lrt -lssl -ldl -lz -lbz2 -ldnnl)
    else()
    target_link_libraries(serving -liomp5 -lmklml_intel -lmkldnn -lpthread -lcrypto -lm -lrt -lssl -ldl -lz -lbz2)
endif()
else()
    target_link_libraries(serving openblas -lpthread -lcrypto -lm -lrt -lssl -ldl -lz -lbz2)
endif()

install(TARGETS serving
        RUNTIME DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/serving/bin)

FILE(GLOB inc ${CMAKE_CURRENT_BINARY_DIR}/*.pb.h)
install(FILES ${inc}
        DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/include/serving)

if (${WITH_MKL})
install(FILES
        ${CMAKE_BINARY_DIR}/third_party/install/Paddle/third_party/install/mklml/lib/libmklml_intel.so
        ${CMAKE_BINARY_DIR}/third_party/install/Paddle/third_party/install/mklml/lib/libiomp5.so
        ${CMAKE_BINARY_DIR}/third_party/install/Paddle/third_party/install/mkldnn/lib/libmkldnn.so.0
        ${CMAKE_BINARY_DIR}/third_party/install/Paddle/third_party/install/mkldnn/lib/libdnnl.so.1
        DESTINATION
        ${PADDLE_SERVING_INSTALL_DIR}/demo/serving/bin)
endif()

if (WITH_ROCM)
    target_link_libraries(serving ${ROCM_HIPRTC_LIB})
endif()
