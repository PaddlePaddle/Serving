FILE(GLOB op_srcs ${CMAKE_CURRENT_LIST_DIR}/*.cpp ${CMAKE_CURRENT_LIST_DIR}/../../predictor/tools/quant.cpp)
LIST(APPEND serving_srcs ${op_srcs})
