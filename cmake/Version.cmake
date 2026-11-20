#------------------------
# Project Version
#------------------------

# Retrieve version from header file
set(PSM_VERSION_HEADER_FILE "${ROOT_DIR}/src/psmoveprotocol/ProtocolVersion.h")

file(STRINGS ${PSM_VERSION_HEADER_FILE} PSM_VERSION_PARTS
  REGEX "^#define[ \t]+PSM_RELEASE_VERSION_(MAJOR|MINOR|BUILD|REVISION)[ \t]+[0-9]+$")

foreach(_part MAJOR MINOR BUILD REVISION)
  string(REGEX MATCH "PSM_RELEASE_VERSION_${_part}[ \t]+[0-9]+" _match "${PSM_VERSION_PARTS}")
  if(NOT _match)
    message(FATAL_ERROR "Unable to retrieve project version from ${PSM_VERSION_HEADER_FILE}")
  endif()
  string(REGEX REPLACE "PSM_RELEASE_VERSION_${_part}[ \t]+([0-9]+)" "\\1" PSM_VERSION_${_part} "${_match}")
endforeach()

set(PSM_VERSION_STRING "${PSM_VERSION_MAJOR}.${PSM_VERSION_MINOR}.${PSM_VERSION_BUILD}.${PSM_VERSION_REVISION}")

message(STATUS "Project version: ${PSM_VERSION_STRING}")
