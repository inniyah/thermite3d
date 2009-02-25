# Find QTOGRE includes and library
#
# This module defines
#  QTOGRE_INCLUDE_DIR
#  QTOGRE_LIBRARIES, the libraries to link against to use QTOGRE.
#  QTOGRE_LIBRARY_DIRS, the location of the libraries
#  QTOGRE_FOUND, If false, do not try to use QTOGRE
#
# Copyright © 2007, Matt Williams
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

IF (QTOGRE_LIBRARIES AND QTOGRE_INCLUDE_DIR)
	SET(QTOGRE_FIND_QUIETLY TRUE) # Already in cache, be silent
ENDIF (QTOGRE_LIBRARIES AND QTOGRE_INCLUDE_DIR)

IF (WIN32) #Windows
	MESSAGE(STATUS "Looking for QtOgre")
	SET(QTOGRE_INCLUDE_DIR "C:\\Program Files\\QtOgreFramework\\include")
	SET(QTOGRE_LIBRARY_DIRS debug "C:\\Program Files\\QtOgreFramework\\lib" optimized "C:\\Program Files (x86)\\QtOgreFramework\\lib")
	SET(QTOGRE_LIBRARIES debug QtOgre_d optimized QtOgre)
ELSE (WIN32) #Unix
	MESSAGE(STATUS "QtOgre check not written for Unix yet")
	#CMAKE_MINIMUM_REQUIRED(VERSION 2.4.7 FATAL_ERROR)
	#FIND_PACKAGE(PkgConfig)
	#PKG_SEARCH_MODULE(OGRE OGRE)
	#SET(OGRE_INCLUDE_DIR ${OGRE_INCLUDE_DIRS})
	#SET(OGRE_LIB_DIR ${OGRE_LIBDIR})
	#SET(OGRE_LIBRARIES ${OGRE_LIBRARIES} CACHE STRING "")
ENDIF (WIN32)

#Do some preparation - currently commented out as it seems to treat spaces as seperators
#This causes problems with windows paths.
#SEPARATE_ARGUMENTS(QTOGRE_INCLUDE_DIR)
#SEPARATE_ARGUMENTS(QTOGRE_LIBRARIES)

SET(QTOGRE_INCLUDE_DIR ${QTOGRE_INCLUDE_DIR} CACHE PATH "")
SET(QTOGRE_LIBRARIES ${QTOGRE_LIBRARIES} CACHE STRING "")
SET(QTOGRE_LIBRARY_DIRS ${QTOGRE_LIBRARY_DIRS} CACHE PATH "")

IF (QTOGRE_INCLUDE_DIR AND QTOGRE_LIBRARIES)
	SET(QTOGRE_FOUND TRUE)
ENDIF (QTOGRE_INCLUDE_DIR AND QTOGRE_LIBRARIES)

IF (QTOGRE_FOUND)
	IF (NOT QTOGRE_FIND_QUIETLY)
		MESSAGE(STATUS "  libraries : ${QTOGRE_LIBRARIES} from ${QTOGRE_LIBRARY_DIRS}")
		MESSAGE(STATUS "  includes  : ${QTOGRE_INCLUDE_DIR}")
	ENDIF (NOT QTOGRE_FIND_QUIETLY)
ELSE (QTOGRE_FOUND)
	IF (QTOGRE_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could not find QTOGRE")
	ENDIF (QTOGRE_FIND_REQUIRED)
ENDIF (QTOGRE_FOUND)
