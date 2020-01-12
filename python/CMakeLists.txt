file(GLOB_RECURSE SERVING_CLIENT_PY_FILES serving_client/*.py)
set(PY_FILES ${SERVING_CLIENT_PY_FILES})
SET(PACKAGE_NAME "serving_client")
set(SETUP_LOG_FILE "setup.py.log")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in
    ${CMAKE_CURRENT_BINARY_DIR}/setup.py)

set(SERVING_CLIENT_CORE ${PADDLE_SERVING_BINARY_DIR}/general-client/serving_client.so)

message("python env: " ${py_env})

add_custom_command(
	OUTPUT ${PADDLE_SERVING_BINARY_DIR}/.timestamp
	COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/paddle_serving/ ${PADDLE_SERVING_BINARY_DIR}/python/
	COMMAND ${CMAKE_COMMAND} -E copy ${PADDLE_SERVING_BINARY_DIR}/general-client/serving_client.so ${PADDLE_SERVING_BINARY_DIR}/python/paddle_serving/serving_client/
	COMMAND env ${py_env} ${PYTHON_EXECUTABLE} setup.py bdist_wheel
	DEPENDS ${SERVING_CLIENT_CORE} sdk_configure_py_proto ${PY_FILES})

add_custom_target(paddle_python ALL DEPENDS ${PADDLE_SERVING_BINARY_DIR}/.timestamp)

set(SERVING_CLIENT_PYTHON_PACKAGE_DIR ${CMAKE_CURRENT_BINARY_DIR}/dist/)

install(DIRECTORY ${SERVING_CLIENT_PYTHON_PACKAGE_DIR}
    DESTINATION opt/serving_client/share/wheels
)

find_program(PATCHELF_EXECUTABLE patchelf)
if(NOT PATCHELF_EXECUTABLE)
  message(FATAL_ERROR "patchelf not found, please install it.\n"
         "For Ubuntu, the command is: apt-get install -y patchelf.")
endif()
