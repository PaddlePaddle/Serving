if (CLIENT_ONLY)
file(GLOB_RECURSE SERVING_CLIENT_PY_FILES serving_client/*.py)
set(PY_FILES ${SERVING_CLIENT_PY_FILES})
SET(PACKAGE_NAME "serving_client")
set(SETUP_LOG_FILE "setup.py.client.log")
endif()

if (NOT CLIENT_ONLY)
file(GLOB_RECURSE SERVING_SERVER_PY_FILES serving_server/*.py)
set(PY_FILES ${SERVING_SERVER_PY_FILES})
SET(PACKAGE_NAME "serving_server")
set(SETUP_LOG_FILE "setup.py.server.log")
endif()

if (CLIENT_ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/setup.py.client.in
    ${CMAKE_CURRENT_BINARY_DIR}/setup.py)
endif()

if (NOT CLIENT_ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/setup.py.server.in
    ${CMAKE_CURRENT_BINARY_DIR}/setup.py)
endif()

set(SERVING_CLIENT_CORE ${PADDLE_SERVING_BINARY_DIR}/core/general-client/serving_client.so)
set(SERVING_SERVER_CORE ${PADDLE_SERVING_BINARY_DIR}/core/predictor/serving_server.so)

message("python env: " ${py_env})

if (CLIENT_ONLY)
add_custom_command(
	OUTPUT ${PADDLE_SERVING_BINARY_DIR}/.timestamp
	COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/paddle_serving/ ${PADDLE_SERVING_BINARY_DIR}/python/
	COMMAND ${CMAKE_COMMAND} -E copy ${PADDLE_SERVING_BINARY_DIR}/core/general-client/serving_client.so ${PADDLE_SERVING_BINARY_DIR}/python/paddle_serving/serving_client/
	COMMAND env ${py_env} ${PYTHON_EXECUTABLE} setup.py bdist_wheel
	DEPENDS ${SERVING_CLIENT_CORE} sdk_configure_py_proto ${PY_FILES})
add_custom_target(paddle_python ALL DEPENDS serving_client ${PADDLE_SERVING_BINARY_DIR}/.timestamp)
endif()

if (NOT CLIENT_ONLY)
add_custom_command(
   	OUTPUT ${PADDLE_SERVING_BINARY_DIR}/.timestamp
	COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/paddle_serving/ ${PADDLE_SERVING_BINARY_DIR}/python/
	COMMAND ${CMAKE_COMMAND} -E copy ${PADDLE_SERVING_BINARY_DIR}/core/predictor/serving_server.so ${PADDLE_SERVING_BINARY_DIR}/python/paddle_serving/serving_server/
	COMMAND env ${py_env} ${PYTHON_EXECUTABLE} setup.py bdist_wheel
	DEPENDS ${SERVING_SERVER_CORE} server_config_py_proto ${PY_FILES})
add_custom_target(paddle_python ALL DEPENDS serving_server ${PADDLE_SERVING_BINARY_DIR}/.timestamp)
endif()

set(SERVING_CLIENT_PYTHON_PACKAGE_DIR ${CMAKE_CURRENT_BINARY_DIR}/dist/)
set(SERVING_SERVER_PYTHON_PACKAGE_DIR ${CMAKE_CURRENT_BINARY_DIR}/dist/)

if (CLIENT_ONLY)
install(DIRECTORY ${SERVING_CLIENT_PYTHON_PACKAGE_DIR}
    DESTINATION opt/serving_client/share/wheels
)
endif()

if (NOT CLIENT_ONLY)
install(DIRECTORY ${SERVING_SERVER_PYTHON_PACKAGE_DIR}
    DESTINATION opt/serving_server/share/wheels
)
endif()

find_program(PATCHELF_EXECUTABLE patchelf)
if(NOT PATCHELF_EXECUTABLE)
  message(FATAL_ERROR "patchelf not found, please install it.\n"
         "For Ubuntu, the command is: apt-get install -y patchelf.")
endif()
