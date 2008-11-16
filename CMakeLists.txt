CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(Thermite2)

#Standard modules
FIND_PACKAGE(Boost 1.34.1 COMPONENTS program_options filesystem REQUIRED)

#Custom modules
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
FIND_PACKAGE(OGRE REQUIRED)
FIND_PACKAGE(PolyVox REQUIRED)
FIND_PACKAGE(QtOgre REQUIRED)

SET(SRC_FILES
	source/main.cpp
	source/ChooseMeshWidget.cpp
	source/DemoGameLogic.cpp
	source/MainMenu.cpp
	source/SurfacePatchRenderable.cpp
	source/TimeStampedRenderOperation.cpp
	source/TimeStampedRenderOperationCache.cpp
	source/TimeStampedSurfacePatchCache.cpp
)

SET(INC_FILES
	include/ChooseMeshWidget.h
	include/DemoGameLogic.h
	include/MainMenu.h
	include/SurfacePatchRenderable.h
	include/TimeStampedRenderOperation.h
	include/TimeStampedRenderOperationCache.h
	include/TimeStampedSurfacePatchCache.h
)

SET(UI_FILES
	ui/ChooseMeshWidget.ui
	ui/MainMenu.ui
)

SET(RESOURCE_FILES
	resources/DemoResources.qrc
)

#"Sources" and "Headers" are the group names in Visual Studio.
SOURCE_GROUP("Sources" FILES ${SRC_FILES})
SOURCE_GROUP("Headers" FILES ${INC_FILES})

FIND_PACKAGE(Qt4)
IF (WIN32)
	SET(QT_USE_QTMAIN 1)
ENDIF (WIN32)
SET(QT_USE_QTGUI 1)
INCLUDE(${QT_USE_FILE})

QT4_WRAP_CPP(MOC_SRCS ${INC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include ${QTOGRE_INCLUDE_DIR} ${OGRE_INCLUDE_DIR} ${POLYVOX_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${OGRE_LIB_DIR} ${QTOGRE_LIB_DIR} ${POLYVOX_LIB_DIR} ${Boost_LIBRARY_DIRS})
ADD_EXECUTABLE(Demo WIN32 ${SRC_FILES}  ${MOC_SRCS} ${UI_SRCS} ${RESOURCE_SRCS})
TARGET_LINK_LIBRARIES(Demo ${QT_LIBRARIES} ${OGRE_LIBRARIES} ${QTOGRE_LIBRARIES} ${POLYVOX_LIBRARIES} ${BOOST_LIBRARIES})

IF (WIN32) #Windows
CONFIGURE_FILE(${OGRE_LIB_DIR}/OgreMain_d.dll ${CMAKE_CURRENT_BINARY_DIR}/debug/OgreMain_d.dll COPYONLY)
CONFIGURE_FILE(${OGRE_LIB_DIR}/RenderSystem_GL_d.dll ${CMAKE_CURRENT_BINARY_DIR}/debug/RenderSystem_GL_d.dll COPYONLY)
CONFIGURE_FILE(${OGRE_LIB_DIR}/RenderSystem_Direct3D9_d.dll ${CMAKE_CURRENT_BINARY_DIR}/debug/RenderSystem_Direct3D9_d.dll COPYONLY)

CONFIGURE_FILE(${OGRE_LIB_DIR}/OgreMain.dll ${CMAKE_CURRENT_BINARY_DIR}/release/OgreMain.dll COPYONLY)
CONFIGURE_FILE(${OGRE_LIB_DIR}/RenderSystem_GL.dll ${CMAKE_CURRENT_BINARY_DIR}/release/RenderSystem_GL.dll COPYONLY)
CONFIGURE_FILE(${OGRE_LIB_DIR}/RenderSystem_Direct3D9.dll ${CMAKE_CURRENT_BINARY_DIR}/release/RenderSystem_Direct3D9.dll COPYONLY)
ENDIF (WIN32)

