cmake_minimum_required(VERSION 3.5)
# The project and its bundled dependencies use legacy CMake compatibility.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
project(PSMoveService VERSION 0.0.1)
include(ExternalProject)

set(ROOT_DIR ${CMAKE_CURRENT_LIST_DIR})

LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
include(cmake/Environment.cmake)
include(cmake/Version.cmake)

set(PSM_PROJECT_NAME "PSMoveService_${PSM_VERSION_STRING}")
set(PSM_ROOT_INSTALL_PATH "${ROOT_DIR}/dist")
set(PSM_DEBUG_INSTALL_PATH "${PSM_ROOT_INSTALL_PATH}/${ARCH_LABEL}/Debug")
set(PSM_RELEASE_INSTALL_PATH "${PSM_ROOT_INSTALL_PATH}/${ARCH_LABEL}/Release")
file(MAKE_DIRECTORY "${PSM_ROOT_INSTALL_PATH}")
file(WRITE "${PSM_ROOT_INSTALL_PATH}/version.txt" "${PSM_PROJECT_NAME}")

include(cmake/ThirdParty.cmake)
include(cmake/Installer.cmake)

# Step into the subdirectories
add_subdirectory(src)

# Stage a ready-to-run folder with the service, the config tool and the client DLL side by side,
# laid out like the release package (same layout as CreateRelease.bat, without the zip and test binaries).
#   cmake --build build --config Release --target stage_release
# Output: <repo>/release/PSMoveService
IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(PSM_STAGE_DIR "${ROOT_DIR}/release/PSMoveService")
    add_custom_target(stage_release
        COMMAND ${CMAKE_COMMAND} -E make_directory "${PSM_STAGE_DIR}/assets/calibration"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:PSMoveService>
            $<TARGET_FILE:PSMoveServiceAdmin>
            $<TARGET_FILE:PSMoveConfigTool>
            $<TARGET_FILE:PSMoveClient_CAPI>
            "${PSM_STAGE_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${ROOT_DIR}/src/psmoveconfigtool/assets" "${PSM_STAGE_DIR}/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${ROOT_DIR}/misc/calibration" "${PSM_STAGE_DIR}/assets/calibration"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ROOT_DIR}/LICENSE" "${PSM_STAGE_DIR}/LICENSE.txt"
        COMMAND ${CMAKE_COMMAND} -E echo "Staged PSMoveService in ${PSM_STAGE_DIR}"
        COMMENT "Staging PSMoveService release folder"
        VERBATIM)
    add_dependencies(stage_release PSMoveService PSMoveServiceAdmin PSMoveConfigTool PSMoveClient_CAPI)
ENDIF()

# TODO: Move tests to root-level
# TODO: CMakeLists for misc (matlab, python)
