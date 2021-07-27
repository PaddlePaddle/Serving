function(PROTOBUF_GENERATE_SERVING_CPP FOR_SERVING_SIDE SRCS HDRS )
  if(NOT ARGN)
      message(SEND_ERROR "Error: PROTOBUF_GENERATE_SERVING_CPP() called without any proto files")
          return()
	    endif()

  if(PROTOBUF_GENERATE_CPP_APPEND_PATH)
    # Create an include path for each file specified
    foreach(FIL ${ARGN})
      get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
      get_filename_component(ABS_PATH ${ABS_FIL} PATH)
      list(FIND _protobuf_include_path ${ABS_PATH} _contains_already)
      if(${_contains_already} EQUAL -1)
          list(APPEND _protobuf_include_path -I ${ABS_PATH})
      endif()
    endforeach()
  else()
    set(_protobuf_include_path -I ${CMAKE_CURRENT_SOURCE_DIR})
  endif()

  if(DEFINED PROTOBUF_IMPORT_DIRS)
      foreach(DIR ${Protobuf_IMPORT_DIRS})
        get_filename_component(ABS_PATH ${DIR} ABSOLUTE)
        list(FIND _protobuf_include_path ${ABS_PATH} _contains_already)
        if(${_contains_already} EQUAL -1)
            list(APPEND _protobuf_include_path -I ${ABS_PATH})
        endif()
      endforeach()
    endif()

    set(${SRCS})
    set(${HDRS})
    foreach(FIL ${ARGN})
      get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
      get_filename_component(FIL_WE ${FIL} NAME_WE)

      list(APPEND ${SRCS} "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.cc")
      list(APPEND ${HDRS} "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.h")

      set(PDCODEGEN "${CMAKE_BINARY_DIR}/core/pdcodegen/pdcodegen")
      if (${FOR_SERVING_SIDE})
          add_custom_command(
              OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.cc"
                     "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.h"
              COMMAND  ${Protobuf_PROTOC_EXECUTABLE}
                  ARGS --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
                  --pdcodegen_out=${CMAKE_CURRENT_BINARY_DIR}
                  --plugin=protoc-gen-pdcodegen=${CMAKE_BINARY_DIR}/core/pdcodegen/pdcodegen
                  --proto_path=${CMAKE_SOURCE_DIR}/core/predictor/proto
                  ${_protobuf_include_path} ${ABS_FIL}
              DEPENDS ${ABS_FIL} ${Protobuf_PROTOC_EXECUTABLE} ${PDCODEGEN}
              COMMENT "Running Paddle-serving C++ protocol buffer compiler on ${FIL}"
              VERBATIM)
      else()
          add_custom_command(
	      OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.cc"
                     "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.pb.h"
              COMMAND  ${Protobuf_PROTOC_EXECUTABLE}
                  ARGS --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
                  --pdcodegen_out=${CMAKE_CURRENT_BINARY_DIR}
                  --plugin=protoc-gen-pdcodegen=${CMAKE_BINARY_DIR}/core/pdcodegen/pdcodegen
                  ${_protobuf_include_path} ${ABS_FIL}
	      DEPENDS ${ABS_FIL} ${Protobuf_PROTOC_EXECUTABLE} ${PDCODEGEN}
              COMMENT "Running Paddle-serving C++ protocol buffer compiler on ${FIL}"
              VERBATIM)
    endif()
  endforeach()

  set_source_files_properties(${${SRCS}} ${${HDRS}} PROPERTIES GENERATED TRUE)
  set(${SRCS} ${${SRCS}} PARENT_SCOPE)
  set(${HDRS} ${${HDRS}} PARENT_SCOPE)
endfunction()

FILE(GLOB protos ${CMAKE_CURRENT_LIST_DIR}/*.proto)
list(APPEND protos ${CMAKE_SOURCE_DIR}/core//predictor/proto/pds_option.proto
        ${CMAKE_SOURCE_DIR}/core//predictor/proto/builtin_format.proto)
PROTOBUF_GENERATE_SERVING_CPP(FALSE PROTO_SRCS PROTO_HDRS ${protos})
LIST(APPEND sdk_cpp_srcs ${PROTO_SRCS})
