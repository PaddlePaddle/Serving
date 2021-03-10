cmake_minimum_required(VERSION 3.2)
project(gpu)
FIND_PACKAGE(CUDA ${CUDA_VERSION} REQUIRED)
SET(CUDA_TARGET_INCLUDE ${CUDA_TOOLKIT_ROOT_DIR}-${CUDA_VERSION}/targets/${CMAKE_HOST_SYSTEM_PROCESSOR}-${LOWER_SYSTEM_NAME}/include)

file(GLOB_RECURSE CURRENT_HEADERS  *.h *.hpp *.cuh)
file(GLOB CURRENT_SOURCES  *.cpp *.cu)

source_group("Include" FILES ${CURRENT_HEADERS}) 
source_group("Source" FILES ${CURRENT_SOURCES}) 

set(CMAKE_CUDA_FLAGS "-ccbin /opt/compiler/gcc-4.8.2/bin -Xcompiler -fPIC --std=c++11")
set(CUDA_NVCC_FLAGS "-L/opt/compiler/gcc-4.8.2/bin -Xcompiler -fPIC --std=c++11")

include_directories(${CUDA_INCLUDE_DIRS})

cuda_add_library(gpu SHARED ${CURRENT_HEADERS} ${CURRENT_SOURCES})
target_link_libraries(gpu ${CUDA_LIBS})
