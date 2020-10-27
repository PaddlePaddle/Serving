cmake_minimum_required(VERSION 3.2)
project(hw-extract)
include(cuda)
include(configure)

execute_process(COMMAND wget -q https://paddle-serving.bj.bcebos.com/external_code/libpreprocess.tar.gz)
execute_process(COMMAND mv libpreprocess.tar.gz ${CMAKE_SOURCE_DIR}/core/preprocess/nvdec-extractframe/)
execute_process(COMMAND tar -xf ${CMAKE_SOURCE_DIR}/core/preprocess/nvdec-extractframe/libpreprocess.tar.gz -C ${CMAKE_SOURCE_DIR}/core/preprocess/nvdec-extractframe/)

#C flags
set(CMAKE_C_FLAGS " -g -pipe -W -Wall -fPIC -Wmissing-field-initializers")

#C++ flags.
set(CMAKE_CXX_FLAGS " -g -pipe -W -Wall -fPIC -std=c++11 -Wmissing-field-initializers")

add_subdirectory(cuda)
set(CMAKE_CUDA_FLAGS "-ccbin -Xcompiler -fPIC --std=c++11")
set(CUDA_NVCC_FLAGS "-Xcompiler -fPIC --std=c++11")
set(EXTRA_LIBS ${EXTRA_LIBS} hwgpu)
set(CUDA_LIBRARIES ${CUDA_TOOLKIT_ROOT_DIR}/lib64)

#release headers
include_directories(*.h)
include_directories(*.hpp)
include_directories(include/*.h)
include_directories(include/*.hpp)

include_directories(.)
include_directories(./include)
include_directories(./codessdkinclude/include)
include_directories(./ffmpeginclude/include)
# include_directories(./pybind11/include)
include_directories(./pythoninclude/python2.7/)
include_directories(./Utils)
include_directories(${CUDA_INCLUDE_DIRS})

# Preprocessor libs.
file(GLOB LIB_FILES ${CMAKE_SOURCE_DIR}/core/preprocess/nvdec-extractframe/lib/*.so)
file(GLOB CUDA_LIB_FILES ${CUDA_LIBRARIES}/*.so)
file(GLOB NVJPEG_LIBS ${CMAKE_SOURCE_DIR}/core/preprocess/nvdec-extractframe/lib/libnvjpeg.*)
file(GLOB SOURCE_FILES src/*.cpp NvDecoder/*.cpp pybind/*.cpp)

#.so
add_library(hwextract SHARED ${SOURCE_FILES})
target_link_libraries(hwextract ${CMAKE_SOURCE_DIR}/core/preprocess/nvdec-extractframe/lib/libnvjpeg.so)
target_link_libraries(hwextract ${EXTRA_LIBS})
target_link_libraries(hwextract ${LIB_FILES})
target_link_libraries(hwextract ${CUDA_LIB_FILES})
# execute_process(COMMAND rm ${CMAKE_SOURCE_DIR}/core/preprocess/nvdec-extractframe/preprocesslib.zip)
