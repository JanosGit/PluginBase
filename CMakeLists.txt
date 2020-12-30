set(JB_PLUGIN_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

add_subdirectory (Ext/json)
juce_add_module (jb_plugin_base)

macro (jb_add_git_version_info TARGET_NAME)
    file (TOUCH ${CMAKE_CURRENT_BINARY_DIR}/gitVersion.cpp)

    add_custom_command (TARGET ${TARGET_NAME}
            PRE_BUILD
            COMMAND ${CMAKE_COMMAND} -DOUT_DIR=${CMAKE_CURRENT_BINARY_DIR} -P ${JB_PLUGIN_BASE_DIR}/CMake/gitVersion.cmake)

    target_compile_definitions (${TARGET_NAME}
        PRIVATE
            JB_HAS_GIT_VERSION=1)
    target_sources (${TARGET_NAME}
        PRIVATE
            ${CMAKE_CURRENT_BINARY_DIR}/gitVersion.cpp)
endmacro()