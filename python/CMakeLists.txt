if (CLIENT)
  file(INSTALL pipeline DESTINATION paddle_serving_client)
  file(GLOB_RECURSE SERVING_CLIENT_PY_FILES paddle_serving_client/*.py)
  set(PY_FILES ${SERVING_CLIENT_PY_FILES})
  SET(PACKAGE_NAME "serving_client")
  set(SETUP_LOG_FILE "setup.py.client.log")
endif()

if (SERVER)
  SET(SERVER_PACKAGE_NAME "paddle-serving-server")
  if (WITH_GPU) 
    set(SERVER_PACKAGE_NAME "paddle-serving-server-gpu")
  elseif(WITH_XPU)
    set(SERVER_PACKAGE_NAME "paddle-serving-server-xpu")
  elseif(WITH_ROCM)
    set(SERVER_PACKAGE_NAME "paddle-serving-server-rocm")
  elseif(WITH_ASCEND_CL)
    set(SERVER_PACKAGE_NAME "paddle-serving-server-npu")
  endif()
  file(INSTALL pipeline DESTINATION paddle_serving_server)
  file(GLOB_RECURSE SERVING_SERVER_PY_FILES paddle_serving_server/*.py)
  set(PY_FILES ${SERVING_SERVER_PY_FILES})
  set(SETUP_LOG_FILE "setup.py.server.log")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/util.py
    ${CMAKE_CURRENT_BINARY_DIR}/util.py)
if (CLIENT)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/setup.py.client.in
    ${CMAKE_CURRENT_BINARY_DIR}/setup.py)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../tools/python_tag.py
    ${CMAKE_CURRENT_BINARY_DIR}/python_tag.py)
endif()

if (APP)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/setup.py.app.in
    ${CMAKE_CURRENT_BINARY_DIR}/setup.py)
endif()

if (SERVER)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/setup.py.server.in
    ${CMAKE_CURRENT_BINARY_DIR}/setup.py)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/gen_version.py
    ${CMAKE_CURRENT_BINARY_DIR}/gen_version.py)

set (SERVING_CLIENT_CORE ${PADDLE_SERVING_BINARY_DIR}/core/general-client/*.so)
message("python env: " ${py_env})

if (APP)
  add_custom_command(
    OUTPUT ${PADDLE_SERVING_BINARY_DIR}/.timestamp
    COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/paddle_serving_app/ ${PADDLE_SERVING_BINARY_DIR}/python/
    COMMAND env ${py_env} ${PYTHON_EXECUTABLE} gen_version.py "app"
    COMMAND env ${py_env} ${PYTHON_EXECUTABLE} setup.py bdist_wheel
    DEPENDS ${SERVING_APP_CORE} general_model_config_py_proto ${PY_FILES})
  add_custom_target(paddle_python ALL DEPENDS ${PADDLE_SERVING_BINARY_DIR}/.timestamp)
endif()

if (CLIENT)
  add_custom_command(
    OUTPUT ${PADDLE_SERVING_BINARY_DIR}/.timestamp
    COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/paddle_serving_client/ ${PADDLE_SERVING_BINARY_DIR}/python/
    COMMAND ${CMAKE_COMMAND} -E copy ${SERVING_CLIENT_CORE} ${PADDLE_SERVING_BINARY_DIR}/python/paddle_serving_client/serving_client.so
    COMMAND env ${py_env} ${PYTHON_EXECUTABLE} python_tag.py
    COMMAND env ${py_env} ${PYTHON_EXECUTABLE} gen_version.py "client"
    COMMAND env ${py_env} ${PYTHON_EXECUTABLE} setup.py bdist_wheel
    DEPENDS ${SERVING_CLIENT_CORE} sdk_configure_py_proto ${PY_FILES})
  add_custom_target(paddle_python ALL DEPENDS serving_client ${PADDLE_SERVING_BINARY_DIR}/.timestamp)
endif()

if (SERVER)
  # todo, generate suffix for cpu、gpu、arm
  if(WITH_TRT)
    if(CUDA_VERSION EQUAL 10.1)
      set(VERSION_SUFFIX 101)
    elseif(CUDA_VERSION EQUAL 10.2)
      if(CUDNN_MAJOR_VERSION EQUAL 7)
        set(VERSION_SUFFIX 102)
      elseif(CUDNN_MAJOR_VERSION EQUAL 8)
        set(VERSION_SUFFIX 1028)
      endif()
    elseif(CUDA_VERSION EQUAL 11.2)
      set(VERSION_SUFFIX 112)
    endif()
  endif()

  if(WITH_LITE)
    set(VERSION_SUFFIX 2)
  endif()
  add_custom_command(
    OUTPUT ${PADDLE_SERVING_BINARY_DIR}/.timestamp
    COMMAND cp -r
    ${CMAKE_CURRENT_SOURCE_DIR}/paddle_serving_server/ ${PADDLE_SERVING_BINARY_DIR}/python/
    COMMAND env ${py_env} ${PYTHON_EXECUTABLE} gen_version.py
    "server" ${VERSION_SUFFIX}
    COMMAND env ${py_env} ${PYTHON_EXECUTABLE} setup.py bdist_wheel
    DEPENDS ${SERVING_SERVER_CORE} server_config_py_proto ${PY_FILES})
  add_custom_target(paddle_python ALL DEPENDS ${PADDLE_SERVING_BINARY_DIR}/.timestamp)
endif()

set(SERVING_CLIENT_PYTHON_PACKAGE_DIR ${CMAKE_CURRENT_BINARY_DIR}/dist/)
set(SERVING_SERVER_PYTHON_PACKAGE_DIR ${CMAKE_CURRENT_BINARY_DIR}/dist/)

if (CLIENT)
  install(DIRECTORY ${SERVING_CLIENT_PYTHON_PACKAGE_DIR}
    DESTINATION opt/serving_client/share/wheels
  )
endif()

if (SERVER)
  install(DIRECTORY ${SERVING_SERVER_PYTHON_PACKAGE_DIR}
      DESTINATION opt/serving_server/share/wheels
  )
endif()

if (CLIENT OR SERVER)
  find_program(PATCHELF_EXECUTABLE patchelf)
  if (NOT PATCHELF_EXECUTABLE)
    message(FATAL_ERROR "patchelf not found, please install it.\n"
           "For Ubuntu, the command is: apt-get install -y patchelf.")
  endif()
endif()
