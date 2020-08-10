if (CLIENT)
    file(INSTALL pipeline DESTINATION paddle_serving_client)
    execute_process(COMMAND ${PYTHON_EXECUTABLE} run_codegen.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/paddle_serving_client/pipeline/proto)
    file(GLOB_RECURSE SERVING_CLIENT_PY_FILES paddle_serving_client/*.py)
    set(PY_FILES ${SERVING_CLIENT_PY_FILES})
    SET(PACKAGE_NAME "serving_client")
    set(SETUP_LOG_FILE "setup.py.client.log")
endif()

if (SERVER)
    if (NOT WITH_GPU)
        file(INSTALL pipeline DESTINATION paddle_serving_server)
        execute_process(COMMAND ${PYTHON_EXECUTABLE} run_codegen.py
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/paddle_serving_server/pipeline/proto)
        file(GLOB_RECURSE SERVING_SERVER_PY_FILES paddle_serving_server/*.py)
    else()
        file(INSTALL pipeline DESTINATION paddle_serving_server_gpu)
        execute_process(COMMAND ${PYTHON_EXECUTABLE} run_codegen.py
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/paddle_serving_server_gpu/pipeline/proto)
        file(GLOB_RECURSE SERVING_SERVER_PY_FILES paddle_serving_server_gpu/*.py)
    endif()
        set(PY_FILES ${SERVING_SERVER_PY_FILES})
        SET(PACKAGE_NAME "serving_server")
        set(SETUP_LOG_FILE "setup.py.server.log")
endif()

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
    if (NOT WITH_GPU)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/setup.py.server.in
            ${CMAKE_CURRENT_BINARY_DIR}/setup.py)
    else()
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/setup.py.server_gpu.in
            ${CMAKE_CURRENT_BINARY_DIR}/setup.py)
    endif()
endif()

set (SERVING_CLIENT_CORE ${PADDLE_SERVING_BINARY_DIR}/core/general-client/*.so)
message("python env: " ${py_env})

if (APP)
add_custom_command(
        OUTPUT ${PADDLE_SERVING_BINARY_DIR}/.timestamp
        COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/paddle_serving_app/ ${PADDLE_SERVING_BINARY_DIR}/python/
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
	COMMAND env ${py_env} ${PYTHON_EXECUTABLE} setup.py bdist_wheel
	DEPENDS ${SERVING_CLIENT_CORE} sdk_configure_py_proto ${PY_FILES})
add_custom_target(paddle_python ALL DEPENDS serving_client ${PADDLE_SERVING_BINARY_DIR}/.timestamp)
endif()

if (SERVER)
    if(NOT WITH_GPU)
        add_custom_command(
            OUTPUT ${PADDLE_SERVING_BINARY_DIR}/.timestamp
            COMMAND cp -r ${CMAKE_CURRENT_SOURCE_DIR}/paddle_serving_server/ ${PADDLE_SERVING_BINARY_DIR}/python/
            COMMAND env ${py_env} ${PYTHON_EXECUTABLE} setup.py bdist_wheel
            DEPENDS ${SERVING_SERVER_CORE} server_config_py_proto ${PY_FILES})
        add_custom_target(paddle_python ALL DEPENDS ${PADDLE_SERVING_BINARY_DIR}/.timestamp)
    else()
        add_custom_command(
            OUTPUT ${PADDLE_SERVING_BINARY_DIR}/.timestamp
            COMMAND cp -r
            ${CMAKE_CURRENT_SOURCE_DIR}/paddle_serving_server_gpu/ ${PADDLE_SERVING_BINARY_DIR}/python/
            COMMAND env ${py_env} ${PYTHON_EXECUTABLE} paddle_serving_server_gpu/gen_cuda_version.py ${CUDA_VERSION_MAJOR}
            COMMAND env ${py_env} ${PYTHON_EXECUTABLE} setup.py bdist_wheel
            DEPENDS ${SERVING_SERVER_CORE} server_config_py_proto ${PY_FILES})
        add_custom_target(paddle_python ALL DEPENDS ${PADDLE_SERVING_BINARY_DIR}/.timestamp)
    endif()
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
