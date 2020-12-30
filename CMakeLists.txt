set(JB_PLUGIN_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

add_subdirectory (Ext/json)
juce_add_module (jb_plugin_base)

# Adds the helper targets jb_create_git_version which will generate a version info source file that contains strings
# describing the current commit hash, commit tag and branch name. This file will be compiled into a tiny static library
# jb_git_version, which the target will then be linked against. A preprocessor flag will trigger the inclusion of the
# GitVersion.h header file into the jb_plugin_base sources
macro (jb_add_git_version_info TARGET_NAME)
    # Some generators don't like if this file doesn't exist right away
    file (TOUCH ${CMAKE_CURRENT_BINARY_DIR}/gitVersion.cpp)

    # This custom target creates the actual file
    add_custom_target (jb_create_git_version
            COMMAND ${CMAKE_COMMAND} -DOUT_DIR=${CMAKE_CURRENT_BINARY_DIR} -P ${JB_PLUGIN_BASE_DIR}/CMake/gitVersion.cmake)

    # This compiles the file into a simple static library
    add_library(jb_git_version STATIC
            ${CMAKE_CURRENT_BINARY_DIR}/gitVersion.cpp)

    # This makes sure that the file is created before it gets compiled
    add_dependencies (jb_git_version jb_create_git_version)

    # If this is set, jb_plugin_base will include GitVersion.h
    target_compile_definitions (jb_git_version
        PUBLIC
            JB_HAS_GIT_VERSION=1)

    target_link_libraries (${TARGET_NAME}
       PRIVATE
            jb_git_version)
endmacro()