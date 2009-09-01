# Find PolyVox includes and library
#
# This module defines
#  PolyVoxCore_INCLUDE_DIR
#  PolyVoxCore_LIBRARIES, the libraries to link against to use PolyVoxCore.
#  PolyVoxCore_LIB_DIR, the location of the libraries
#  PolyVoxCore_FOUND, If false, do not try to use PolyVoxCore
#  PolyVoxUtil_INCLUDE_DIR
#  PolyVoxUtil_LIBRARIES, the libraries to link against to use PolyVoxUtil.
#  PolyVoxUtil_LIB_DIR, the location of the libraries
#  PolyVoxUtil_FOUND, If false, do not try to use PolyVoxUtil
#
# Copyright © 2007, Matt Williams
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

IF (PolyVoxCore_LIBRARIES AND PolyVoxCore_INCLUDE_DIRS AND PolyVoxUtil_LIBRARIES AND PolyVoxUtil_INCLUDE_DIRS)
	SET(PolyVox_FIND_QUIETLY TRUE) # Already in cache, be silent
ENDIF (PolyVoxCore_LIBRARIES AND PolyVoxCore_INCLUDE_DIRS AND PolyVoxUtil_LIBRARIES AND PolyVoxUtil_INCLUDE_DIRS)

IF (WIN32) #Windows
	MESSAGE(STATUS "Looking for PolyVox")
	SET(PolyVox_HOME_EXISTS $ENV{POLYVOX_HOME})
	IF (PolyVox_HOME_EXISTS)
		SET(PolyVoxCore_INCLUDE_DIRS $ENV{POLYVOX_HOME}/PolyVoxCore/include)
		SET(PolyVoxCore_LIBRARY_DIRS $ENV{POLYVOX_HOME}/PolyVoxCore/lib)
		SET(PolyVoxCore_LIBRARIES debug PolyVoxCore_d optimized PolyVoxCore)
		SET(PolyVoxUtil_INCLUDE_DIRS $ENV{POLYVOX_HOME}/PolyVoxUtil/include)
		SET(PolyVoxUtil_LIBRARY_DIRS $ENV{POLYVOX_HOME}/PolyVoxUtil/lib)
		SET(PolyVoxUtil_LIBRARIES debug PolyVoxUtil_d optimized PolyVoxUtil)
	ENDIF (PolyVox_HOME_EXISTS)
ELSE (WIN32) #Unix
	MESSAGE(STATUS "PolyVox check not written for Unix yet")
	#CMAKE_MINIMUM_REQUIRED(VERSION 2.4.7 FATAL_ERROR)
	#FIND_PACKAGE(PkgConfig)
	#PKG_SEARCH_MODULE(OGRE OGRE)
	#SET(OGRE_INCLUDE_DIR ${OGRE_INCLUDE_DIRS})
	#SET(OGRE_LIB_DIR ${OGRE_LIBDIR})
	#SET(OGRE_LIBRARIES ${OGRE_LIBRARIES} CACHE STRING "")
ENDIF (WIN32)

#Do some preparation
#SEPARATE_ARGUMENTS(PolyVox_INCLUDE_DIR)
#SEPARATE_ARGUMENTS(PolyVox_LIBRARIES)

SET(PolyVoxCore_INCLUDE_DIRS ${PolyVoxCore_INCLUDE_DIRS} CACHE PATH "")
SET(PolyVoxCore_LIBRARIES ${PolyVoxCore_LIBRARIES} CACHE STRING "")
SET(PolyVoxCore_LIBRARY_DIRS ${PolyVoxCore_LIBRARY_DIRS} CACHE PATH "")

SET(PolyVoxUtil_INCLUDE_DIRS ${PolyVoxUtil_INCLUDE_DIRS} CACHE PATH "")
SET(PolyVoxUtil_LIBRARIES ${PolyVoxUtil_LIBRARIES} CACHE STRING "")
SET(PolyVoxUtil_LIBRARY_DIRS ${PolyVoxUtil_LIBRARY_DIRS} CACHE PATH "")

IF (PolyVoxCore_LIBRARIES AND PolyVoxCore_INCLUDE_DIRS AND PolyVoxUtil_LIBRARIES AND PolyVoxUtil_INCLUDE_DIRS)
	SET(PolyVox_FOUND TRUE)
ENDIF (PolyVoxCore_LIBRARIES AND PolyVoxCore_INCLUDE_DIRS AND PolyVoxUtil_LIBRARIES AND PolyVoxUtil_INCLUDE_DIRS)

IF (PolyVox_FOUND)
	IF (NOT PolyVox_FIND_QUIETLY)
		MESSAGE(STATUS "  libraries : ${PolyVoxCore_LIBRARIES} from ${PolyVoxCore_LIBRARY_DIRS} and ${PolyVoxUtil_LIBRARIES} from ${PolyVoxUtil_LIBRARY_DIRS}")
		MESSAGE(STATUS "  includes  : ${PolyVoxCore_INCLUDE_DIRS} and ${PolyVoxUtil_INCLUDE_DIRS}")
	ENDIF (NOT PolyVox_FIND_QUIETLY)
ELSE (PolyVox_FOUND)
	IF (PolyVox_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could not find PolyVox")
	ENDIF (PolyVox_FIND_REQUIRED)
ENDIF (PolyVox_FOUND)
