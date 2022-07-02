if (SERVER OR CLIENT)
  LIST(APPEND protofiles
          ${CMAKE_CURRENT_LIST_DIR}/proto/server_configure.proto
          ${CMAKE_CURRENT_LIST_DIR}/proto/sdk_configure.proto
          ${CMAKE_CURRENT_LIST_DIR}/proto/inferencer_configure.proto
  	${CMAKE_CURRENT_LIST_DIR}/proto/general_model_config.proto
  )
  
  PROTOBUF_GENERATE_CPP(configure_proto_srcs configure_proto_hdrs ${protofiles})
  list(APPEND configure_srcs ${configure_proto_srcs})
  
  list(APPEND configure_srcs ${CMAKE_CURRENT_LIST_DIR}/src/configure_parser.cpp)
  
  add_library(configure ${configure_srcs})
  add_dependencies(configure brpc)
  
  install(TARGETS configure 
          ARCHIVE DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/lib
          )
  
  install(FILES ${CMAKE_CURRENT_LIST_DIR}/include/configure_parser.h
          DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/include/configure/include)
  
  FILE(GLOB inc ${CMAKE_CURRENT_BINARY_DIR}/*.pb.h)
  
  install(FILES ${inc}
          DESTINATION ${PADDLE_SERVING_INSTALL_DIR}/include/configure)
endif()

if (WITH_PYTHON)

  py_proto_compile(general_model_config_py_proto SRCS proto/general_model_config.proto)
  add_custom_target(general_model_config_py_proto_init ALL COMMAND ${CMAKE_COMMAND} -E touch __init__.py)
  add_dependencies(general_model_config_py_proto general_model_config_py_proto_init)
  
  py_grpc_proto_compile(general_model_service_py_proto SRCS proto/general_model_service.proto)
  add_custom_target(general_model_service_py_proto_init ALL COMMAND ${CMAKE_COMMAND} -E touch __init__.py)
  add_dependencies(general_model_service_py_proto general_model_service_py_proto_init)
  
  if (CLIENT)
    py_proto_compile(sdk_configure_py_proto SRCS proto/sdk_configure.proto)
    add_custom_target(sdk_configure_py_proto_init ALL COMMAND ${CMAKE_COMMAND} -E touch __init__.py)
    add_dependencies(sdk_configure_py_proto sdk_configure_py_proto_init)
    add_custom_command(TARGET sdk_configure_py_proto POST_BUILD
    		COMMAND ${CMAKE_COMMAND} -E make_directory ${PADDLE_SERVING_BINARY_DIR}/python/paddle_serving_client/proto
    		COMMAND cp -f *.py ${PADDLE_SERVING_BINARY_DIR}/python/paddle_serving_client/proto
    		COMMENT "Copy generated python proto into directory paddle_serving_client/proto."
    		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    
    add_custom_command(TARGET general_model_config_py_proto POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${PADDLE_SERVING_BINARY_DIR}/python/paddle_serving_client/proto
                    COMMAND cp -f *.py ${PADDLE_SERVING_BINARY_DIR}/python/paddle_serving_client/proto
                    COMMENT "Copy generated general_model_config proto file into directory paddle_serving_client/proto."
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

    add_custom_command(TARGET general_model_service_py_proto POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${PADDLE_SERVING_BINARY_DIR}/python/paddle_serving_client/proto
                    COMMAND cp -f *.py ${PADDLE_SERVING_BINARY_DIR}/python/paddle_serving_client/proto
                    COMMENT "Copy generated general_model_service proto file into directory paddle_serving_client/proto."
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

  endif()
  
  if (APP)
    add_custom_command(TARGET general_model_config_py_proto POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${PADDLE_SERVING_BINARY_DIR}/python/paddle_serving_app/proto
                    COMMAND cp -f *.py ${PADDLE_SERVING_BINARY_DIR}/python/paddle_serving_app/proto
                    COMMENT "Copy generated general_model_config proto file into directory paddle_serving_app/proto."
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  endif()
  
  if (SERVER)
    py_proto_compile(server_config_py_proto SRCS proto/server_configure.proto)
    add_custom_target(server_config_py_proto_init ALL COMMAND ${CMAKE_COMMAND} -E touch __init__.py)
    add_dependencies(server_config_py_proto server_config_py_proto_init)
    add_custom_command(TARGET server_config_py_proto POST_BUILD
    		COMMAND ${CMAKE_COMMAND} -E make_directory ${PADDLE_SERVING_BINARY_DIR}/python/paddle_serving_server/proto
    		COMMAND cp -f *.py ${PADDLE_SERVING_BINARY_DIR}/python/paddle_serving_server/proto
    		COMMENT "Copy generated python proto into directory paddle_serving_server/proto."
    		WORKING_DIRECTORY ${CMAKE_CURRENT_BINRARY_DIR})
    
    add_custom_command(TARGET general_model_config_py_proto POST_BUILD
    		COMMAND ${CMAKE_COMMAND} -E make_directory ${PADDLE_SERVING_BINARY_DIR}/python/paddle_serving_server/proto
    		COMMAND cp -f *.py ${PADDLE_SERVING_BINARY_DIR}/python/paddle_serving_server/proto
    		COMMENT "Copy generated general_model_config proto file into directory paddle_serving_server/proto."
    		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

    add_custom_command(TARGET general_model_service_py_proto POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${PADDLE_SERVING_BINARY_DIR}/python/paddle_serving_server/proto
                    COMMAND cp -f *.py ${PADDLE_SERVING_BINARY_DIR}/python/paddle_serving_server/proto
                    COMMENT "Copy generated general_model_service proto file into directory paddle_serving_server/proto."
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    
  endif()

endif()
