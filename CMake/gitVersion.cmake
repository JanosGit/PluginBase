# Inspired by https://www.mattkeeter.com/blog/2018-01-06-versioning/

execute_process(COMMAND git log --pretty=format:'%h' -n 1
        OUTPUT_VARIABLE GIT_REV
        ERROR_QUIET)

# Check whether we got any revision (which isn't
# always the case, e.g. when someone downloaded a zip
# file from Github instead of a checkout)
if ("${GIT_REV}" STREQUAL "")
    set(GIT_REV "N/A")
    set(GIT_TAG "N/A")
    set(GIT_BRANCH "N/A")
else()
    execute_process(
            COMMAND git describe --exact-match --tags
            OUTPUT_VARIABLE GIT_TAG ERROR_QUIET)
    execute_process(
            COMMAND git rev-parse --abbrev-ref HEAD
            OUTPUT_VARIABLE GIT_BRANCH)

    string (STRIP "${GIT_REV}" GIT_REV)
    string (SUBSTRING "${GIT_REV}" 1 7 GIT_REV)
    string (STRIP "${GIT_TAG}" GIT_TAG)
    string (STRIP "${GIT_BRANCH}" GIT_BRANCH)
endif()

set (VERSION "// Auto generated file

#include <string>
#include \"${CMAKE_CURRENT_LIST_DIR}/../jb_plugin_base/Utils/GitVersion.h\"

namespace ProjectInfo
{
    const std::string Git::commit = \"${GIT_REV}\";
    const std::string Git::tag    = \"${GIT_TAG}\";
    const std::string Git::branch = \"${GIT_BRANCH}\";
}")

if (EXISTS ${OUT_DIR}/gitVersion.cpp)
    file (READ ${OUT_DIR}/gitVersion.cpp PREVIOUS_VERSION)
else()
    set(PREVIOUS_VERSION "")
endif()

if (NOT "${VERSION}" STREQUAL "${PREVIOUS_VERSION}")
    file (WRITE ${OUT_DIR}/gitVersion.cpp "${VERSION}")
    message(STATUS "Updated gitVersion.cpp")
else()
    message(STATUS "gitVersion.cpp is up to date")
endif()
