configure_file("${CMAKE_CURRENT_SOURCE_DIR}/switch/version.h.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/switch/version.h" @ONLY)


set (CSA_IR_SRCS
     ${CMAKE_CURRENT_SOURCE_DIR}/ir/dbprint-csa-ext.cpp
     ${CMAKE_CURRENT_SOURCE_DIR}/ir/ir-csa-ext.cpp
    )
set(EXTENSION_FRONTEND_SOURCES ${EXTENSION_FRONTEND_SOURCES} ${CSA_IR_SRCS} PARENT_SCOPE)


# sources for backend executable
set (CSA_SWITCH_SRCS
    switch/main.cpp
    switch/options.h
    switch/parseInput.h
    switch/parseInput.cpp
    midend/csamidend.h
    midend/csamidend.cpp
    midend/controlStateReconInfo.h
    midend/controlStateReconInfo.cpp
    midend/parserConverter.h
    midend/parserConverter.cpp
    midend/deparserConverter.h
    midend/deparserConverter.cpp
    midend/mergeDeclarations.h
    midend/mergeDeclarations.cpp
    midend/slicePipeControl.h
    midend/slicePipeControl.cpp
    midend/toControl.h
    midend/toControl.cpp
    midend/toV1Model.h
    midend/toV1Model.cpp
    midend/csaExternSubstituter.h
    midend/csaExternSubstituter.cpp
    )
add_cpplint_files (${CMAKE_CURRENT_SOURCE_DIR} "${CSA_SWITCH_SRCS}")


set (P4C_CSA_EXT_HEADERS p4include/csa.p4)
set (IR_DEF_FILES ${IR_DEF_FILES} ${CMAKE_CURRENT_SOURCE_DIR}/ir/csa.def PARENT_SCOPE)

build_unified(CSA_SWITCH_SRCS)
add_executable(p4c-csa ${CSA_SWITCH_SRCS})
target_link_libraries (p4c-csa ${P4C_LIBRARIES} ${P4C_LIB_DEPS})

install(TARGETS p4c-csa RUNTIME DESTINATION ${P4C_RUNTIME_OUTPUT_DIRECTORY})
install (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/p4include DESTINATION ${P4C_ARTIFACTS_OUTPUT_DIRECTORY})


add_custom_target(linkp4ccsa
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_BINARY_DIR}/p4c-csa ${P4C_BINARY_DIR}/p4c-csa
    COMMAND ${CMAKE_COMMAND} -E make_directory ${P4C_BINARY_DIR}/p4include &&
            ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${P4C_CSA_EXT_HEADERS} ${P4C_BINARY_DIR}/p4include
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${P4C_BINARY_DIR}/p4include ${CMAKE_CURRENT_BINARY_DIR}/p4include
    )

add_dependencies(p4c_driver linkp4ccsa)