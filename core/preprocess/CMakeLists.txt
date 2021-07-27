if(NOT WITH_GPU)
    message("GPU preprocess will not be compiled.")
    return()
endif()

message(STATUS "CUDA detected: " ${CUDA_VERSION})
if (${CUDA_VERSION} LESS 10.0)
    message("CUDA version should be 10.0.")
    return()
elseif (${CUDA_VERSION} LESS 10.1) # CUDA 10.0
   add_subdirectory(hwvideoframe)
endif()
