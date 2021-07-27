cmake_minimum_required(VERSION 3.2)
project(gpupreprocess)
include(cuda)
include(configure)

#C flags
set(CMAKE_C_FLAGS " -g -pipe -W -Wall -fPIC")

#C++ flags.
set(CMAKE_CXX_FLAGS " -g -pipe -W -Wall -fPIC -std=c++11")

add_subdirectory(cuda)
set(PYTHON_SO {PYTHON_LIBRARY})
set(EXTRA_LIBS ${EXTRA_LIBS} gpu)
file(GLOB SOURCE_FILES pybind/*.cpp src/*.cpp)

include_directories("./include")
include_directories(${CUDA_INCLUDE_DIRS})
include_directories(/usr${PYTHON_INCLUDE_DIR})

link_directories("-L${CUDA_TOOLKIT_ROOT_DIR}/lib64 -lcudart -lnppidei_static -lnppial_static -lnpps_static -lnppc_static -lculibos")
link_directories(${PYTHON_SO})

#.so
add_library(gpupreprocess SHARED ${SOURCE_FILES})
target_link_libraries(gpupreprocess ${EXTRA_LIBS})
target_link_libraries(gpupreprocess ${CUDA_LIBRARIES})
