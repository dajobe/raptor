find_path(YAJL_INCLUDE_DIR
  NAMES yajl/yajl_parse.h
  HINTS ${YAJL_ROOT} ENV YAJL_ROOT
  PATH_SUFFIXES include
)

find_library(YAJL_LIBRARY
  NAMES yajl
  HINTS ${YAJL_ROOT} ENV YAJL_ROOT
  PATH_SUFFIXES lib
)

# Determine the YAJL major API version (1 or 2) from yajl_version.h
if(YAJL_INCLUDE_DIR AND EXISTS "${YAJL_INCLUDE_DIR}/yajl/yajl_version.h")
  file(STRINGS "${YAJL_INCLUDE_DIR}/yajl/yajl_version.h" _yajl_major_line
       REGEX "^#define[ \t]+YAJL_MAJOR[ \t]+[0-9]+")
  string(REGEX REPLACE "^#define[ \t]+YAJL_MAJOR[ \t]+([0-9]+).*" "\\1"
         YAJL_VERSION_MAJOR "${_yajl_major_line}")
  unset(_yajl_major_line)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(YAJL DEFAULT_MSG YAJL_LIBRARY YAJL_INCLUDE_DIR)

set(YAJL_LIBRARIES ${YAJL_LIBRARY})
set(YAJL_INCLUDE_DIRS ${YAJL_INCLUDE_DIR})

mark_as_advanced(YAJL_INCLUDE_DIR YAJL_LIBRARY)
