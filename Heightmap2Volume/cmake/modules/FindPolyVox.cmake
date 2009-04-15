# Find PolyVox includes and library
#
# This module defines
#  POLYVOX_INCLUDE_DIR
#  POLYVOX_LIBRARIES, the libraries to link against to use PolyVox.
#  POLYVOX_LIB_DIR, the location of the libraries
#  POLYVOX_FOUND, If false, do not try to use PolyVox
#
# Copyright © 2007, Matt Williams
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

IF (POLYVOX_LIBRARIES AND POLYVOX_INCLUDE_DIR)
	SET(POLYVOX_FIND_QUIETLY TRUE) # Already in cache, be silent
ENDIF (POLYVOX_LIBRARIES AND POLYVOX_INCLUDE_DIR)

IF (WIN32) #Windows
	MESSAGE(STATUS "Looking for PolyVox")
	SET(POLYVOX_HOME_EXISTS $ENV{POLYVOX_HOME})
	IF (POLYVOX_HOME_EXISTS)
		SET(POLYVOX_INCLUDE_DIR $ENV{POLYVOX_HOME}/include)
		SET(POLYVOX_LIB_DIR $ENV{POLYVOX_HOME}/lib)
		SET(POLYVOX_LIBRARIES debug PolyVoxCore_d debug PolyVoxUtil_d optimized PolyVoxCore optimized PolyVoxUtil)
	ENDIF (POLYVOX_HOME_EXISTS)
ELSE (WIN32) #Unix
	#CMAKE_MINIMUM_REQUIRED(VERSION 2.4.7 FATAL_ERROR)
	#FIND_PACKAGE(PkgConfig)
	#PKG_SEARCH_MODULE(OGRE OGRE)
	#SET(OGRE_INCLUDE_DIR ${OGRE_INCLUDE_DIRS})
	#SET(OGRE_LIB_DIR ${OGRE_LIBDIR})
	#SET(OGRE_LIBRARIES ${OGRE_LIBRARIES} CACHE STRING "")
ENDIF (WIN32)

#Do some preparation
#SEPARATE_ARGUMENTS(POLYVOX_INCLUDE_DIR)
#SEPARATE_ARGUMENTS(POLYVOX_LIBRARIES)

SET(POLYVOX_INCLUDE_DIR ${POLYVOX_INCLUDE_DIR} CACHE PATH "")
SET(POLYVOX_LIBRARIES ${POLYVOX_LIBRARIES} CACHE STRING "")
SET(POLYVOX_LIB_DIR ${POLYVOX_LIB_DIR} CACHE PATH "")

IF (POLYVOX_INCLUDE_DIR AND POLYVOX_LIBRARIES)
	SET(POLYVOX_FOUND TRUE)
ENDIF (POLYVOX_INCLUDE_DIR AND POLYVOX_LIBRARIES)

IF (POLYVOX_FOUND)
	IF (NOT POLYVOX_FIND_QUIETLY)
		MESSAGE(STATUS "  libraries : ${POLYVOX_LIBRARIES} from ${POLYVOX_LIB_DIR}")
		MESSAGE(STATUS "  includes  : ${POLYVOX_INCLUDE_DIR}")
	ENDIF (NOT POLYVOX_FIND_QUIETLY)
ELSE (POLYVOX_FOUND)
	IF (POLYVOX_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could not find PolyVox")
	ENDIF (POLYVOX_FIND_REQUIRED)
ENDIF (POLYVOX_FOUND)
