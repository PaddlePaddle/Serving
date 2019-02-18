find_library(MKLML_LIBS NAMES libmklml_intel.so libiomp5.so)
include(op/CMakeLists.txt)
include(proto/CMakeLists.txt)
add_executable(image_class ${serving_srcs})
add_dependencies(image_class pdcodegen fluid_cpu_engine pdserving paddle_fluid)
target_link_libraries(image_class -Wl,--whole-archive fluid_cpu_engine
        -Wl,--no-whole-archive pdserving paddle_fluid ${paddle_depend_libs}
        ${MKLML_LIB} ${MKLML_IOMP_LIB}  -lpthread -lcrypto -lm -lrt -lssl -ldl
        -lz)
