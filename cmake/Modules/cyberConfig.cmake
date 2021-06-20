if(NOT PKG_CONFIG_FOUND)
    INCLUDE(FindPkgConfig)
endif()
PKG_CHECK_MODULES(PC_CYBER cyber)

FIND_PATH(
    CYBER_INCLUDE_DIRS
    NAMES cyber/api.h
    HINTS $ENV{CYBER_DIR}/include
        ${PC_CYBER_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    CYBER_LIBRARIES
    NAMES gnuradio-cyber
    HINTS $ENV{CYBER_DIR}/lib
        ${PC_CYBER_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
          )

include("${CMAKE_CURRENT_LIST_DIR}/cyberTarget.cmake")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CYBER DEFAULT_MSG CYBER_LIBRARIES CYBER_INCLUDE_DIRS)
MARK_AS_ADVANCED(CYBER_LIBRARIES CYBER_INCLUDE_DIRS)
