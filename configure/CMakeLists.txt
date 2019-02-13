LIST(APPEND configure_srcs
    ${CMAKE_CURRENT_LIST_DIR}/FileReloader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ConfigReloader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/CmdOption.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ConfigError.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ConfigGroup.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ConfigUnit.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Configure.cpp
    ${CMAKE_CURRENT_LIST_DIR}/constraint/Constraint.cpp
    ${CMAKE_CURRENT_LIST_DIR}/constraint/ConstraintFunction.cpp
    ${CMAKE_CURRENT_LIST_DIR}/constraint/cc_default.cpp
    ${CMAKE_CURRENT_LIST_DIR}/reader/RawData.cpp
    ${CMAKE_CURRENT_LIST_DIR}/reader/Reader.cpp
    # ${CMAKE_CURRENT_LIST_DIR}/utils/init.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utils/trans.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cfgext.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cfgflag.cpp
    ${CMAKE_CURRENT_LIST_DIR}/compiler/src/idl.c
    ${CMAKE_CURRENT_LIST_DIR}/compiler/src/idl_lex.c
    ${CMAKE_CURRENT_LIST_DIR}/compiler/src/idl_gram.c
)

SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_LIST_DIR}/compiler/src/idl.c
        ${CMAKE_CURRENT_LIST_DIR}/compiler/src/idl_lex.c
        ${CMAKE_CURRENT_LIST_DIR}/compiler/src/idl_gram.c PROPERTIES LANGUAGE CXX)

add_library(configure ${configure_srcs})
add_dependencies(configure bsl brpc ullib spreg)
target_include_directories(configure PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/
        ${CMAKE_CURRENT_LIST_DIR}/compiler/src
        ${CMAKE_CURRENT_LIST_DIR}/../ullib/include
        ${CMAKE_CURRENT_LIST_DIR}/../spreg/
        ${CMAKE_CURRENT_BINARY_DIR}/../bsl/include)
