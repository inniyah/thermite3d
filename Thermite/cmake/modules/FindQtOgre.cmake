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

IF (QtOgre_LIBRARIES AND QtOgre_INCLUDE_DIRS)
	SET(QtOgre_FIND_QUIETLY TRUE) # Already in cache, be silent
ENDIF ()

IF (WIN32) #Windows
	MESSAGE(STATUS "Looking for QtOgre")
	SET(QtOgre_INCLUDE_DIRS "C:\\Program Files\\QtOgreFramework\\include")
	SET(QtOgre_LIBRARY_DIRS debug "C:\\Program Files\\QtOgreFramework\\lib" optimized "C:\\Program Files (x86)\\QtOgreFramework\\lib")
	SET(QtOgre_LIBRARIES debug QtOgre_d optimized QtOgre)
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

SET(QtOgre_INCLUDE_DIRS ${QtOgre_INCLUDE_DIRS} CACHE PATH "")
SET(QtOgre_LIBRARIES ${QtOgre_LIBRARIES} CACHE STRING "")
SET(QtOgre_LIBRARY_DIRS ${QtOgre_LIBRARY_DIRS} CACHE PATH "")

IF (QtOgre_INCLUDE_DIRS AND QtOgre_LIBRARIES)
	SET(QtOgre_FOUND TRUE)
ENDIF ()

IF (QtOgre_FOUND)
	IF (NOT QtOgre_FIND_QUIETLY)
		MESSAGE(STATUS "  libraries : ${QtOgre_LIBRARIES} from ${QtOgre_LIBRARY_DIRS}")
		MESSAGE(STATUS "  includes  : ${QtOgre_INCLUDE_DIRS}")
	ENDIF ()
ELSE ()
	IF (QtOgre_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could not find QtOgre")
	ENDIF ()
ENDIF ()
