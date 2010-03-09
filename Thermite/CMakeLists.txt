CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(Thermite)

#Standard modules
#FIND_PACKAGE(Boost 1.34.1 COMPONENTS program_options filesystem REQUIRED)

OPTION(ENABLE_BULLET_PHYSICS "Should we enable Bullet for physics simulation?" ON) #Disable with -DENABLE_BULLET_PHYSICS=OFF or in the GUI
IF(ENABLE_BULLET_PHYSICS)
	MESSAGE(STATUS "Bullet physics enabled")
ELSE()
	MESSAGE(STATUS "Bullet physics disabled")
ENDIF()

#Custom modules
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
IF(ENABLE_BULLET_PHYSICS)
	FIND_PACKAGE(Bullet REQUIRED)
ENDIF()
FIND_PACKAGE(OGRE 1.6 REQUIRED)
IF(ENABLE_BULLET_PHYSICS)
	FIND_PACKAGE(OGREBullet REQUIRED)
ENDIF()
FIND_PACKAGE(PolyVox REQUIRED)
FIND_PACKAGE(QtOgre REQUIRED)

SET(SRC_FILES
	source/main.cpp
	source/AnyOption.cpp
	source/ApplicationGameLogic.cpp
	source/CannonController.cpp
	source/DataStreamWrapper.cpp
	source/LoadingProgress.cpp
	source/LoadMapWidget.cpp
	source/MainMenu.cpp
	source/Map.cpp
	source/MapHandler.cpp
	source/MapManager.cpp
	source/MapRegion.cpp
	source/MapResource.cpp
	source/PhysicalEntity.cpp
	source/Shell.cpp	
	source/SurfacePatchRenderable.cpp
	source/ThermiteGameLogic.cpp
	source/Utility.cpp
	source/VolumeManager.cpp
	source/VolumeResource.cpp
	source/VolumeSerializationProgressListenerImpl.cpp
	
	source/tasks/SurfaceMeshDecimationTask.cpp
	source/tasks/SurfaceMeshExtractionTask.cpp
	source/tasks/SurfaceExtractorTaskData.cpp
	source/tasks/Task.cpp
	source/tasks/TaskProcessorThread.cpp
)

SET(INC_FILES
	include/AnyOption.h
	include/ApplicationGameLogic.h
	include/CannonController.h
	include/DataStreamWrapper.h
	include/LoadingProgress.h
	include/LoadMapWidget.h
	include/MainMenu.h
	include/Map.h
	include/MapHandler.h
	include/MapManager.h
	include/MapRegion.h
	include/MapResource.h
	include/PhysicalEntity.h
	include/Shell.h
	include/SurfacePatchRenderable.h
	include/ThermiteForwardDeclarations.h
	include/ThermiteGameLogic.h
	include/Utility.h
	include/VolumeManager.h
	include/VolumeResource.h
	include/VolumeSerializationProgressListenerImpl.h
	
	include/tasks/SurfaceMeshDecimationTask.h
	include/tasks/SurfaceMeshExtractionTask.h
	include/tasks/SurfaceExtractorTaskData.h
	include/tasks/Task.h
	include/tasks/TaskProcessorThread.h

)

SET(UI_FILES
	ui/CannonController.ui
	ui/LoadingProgress.ui
	ui/LoadMapWidget.ui
	ui/MainMenu.ui
)

SET(RESOURCE_FILES
	resources/Qt/ThermiteResources.qrc
)

#"Sources" and "Headers" are the group names in Visual Studio.
SOURCE_GROUP("Sources" FILES ${SRC_FILES})
SOURCE_GROUP("Headers" FILES ${INC_FILES})

FIND_PACKAGE(Qt4)
IF (WIN32)
	SET(QT_USE_QTMAIN 1)
ENDIF (WIN32)
SET(QT_USE_QTGUI 1)
SET(QT_USE_QTXML 1)
INCLUDE(${QT_USE_FILE})

QT4_WRAP_CPP(MOC_SRCS ${INC_FILES})
QT4_WRAP_UI(UI_SRCS ${UI_FILES})
QT4_ADD_RESOURCES(RESOURCE_SRCS ${RESOURCE_FILES})

IF(ENABLE_BULLET_PHYSICS)
	ADD_DEFINITIONS(-DENABLE_BULLET_PHYSICS)
	INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include/tasks ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/include ${CMAKE_CURRENT_BINARY_DIR}/include/tasks ${QtOgre_INCLUDE_DIRS} ${OGRE_INCLUDE_DIRS} ${PolyVoxCore_INCLUDE_DIRS} ${PolyVoxUtil_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} ${BULLET_INCLUDE_DIRS} ${OGREBULLET_INCLUDE_DIR})
	LINK_DIRECTORIES(${OGRE_LIBRARY_DIRS} ${QtOgre_LIBRARY_DIRS} ${PolyVoxCore_LIBRARY_DIRS} ${PolyVoxUtil_LIBRARY_DIRS} ${Boost_LIBRARY_DIRS} ${BULLET_LIBRARY_DIRS} ${OGREBULLET_LIB_DIR})
	ADD_EXECUTABLE(Thermite WIN32 ${SRC_FILES}  ${MOC_SRCS} ${UI_SRCS} ${RESOURCE_SRCS})
	TARGET_LINK_LIBRARIES(Thermite ${QT_LIBRARIES} ${OGRE_LIBRARIES} ${QtOgre_LIBRARIES} ${PolyVoxCore_LIBRARIES} ${PolyVoxUtil_LIBRARIES} ${BOOST_LIBRARIES} ${OGREBULLET_LIBRARIES} ${BULLET_LIBRARIES})
ELSE()
	INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/include/tasks ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/include ${CMAKE_CURRENT_BINARY_DIR}/include/tasks ${QtOgre_INCLUDE_DIRS} ${OGRE_INCLUDE_DIRS} ${PolyVoxCore_INCLUDE_DIRS} ${PolyVoxUtil_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
	LINK_DIRECTORIES(${OGRE_LIBRARY_DIRS} ${QtOgre_LIBRARY_DIRS} ${PolyVoxCore_LIBRARY_DIRS} ${PolyVoxUtil_LIBRARY_DIRS} ${Boost_LIBRARY_DIRS})
	ADD_EXECUTABLE(Thermite WIN32 ${SRC_FILES}  ${MOC_SRCS} ${UI_SRCS} ${RESOURCE_SRCS})
	TARGET_LINK_LIBRARIES(Thermite ${QT_LIBRARIES} ${OGRE_LIBRARIES} ${QtOgre_LIBRARIES} ${PolyVoxCore_LIBRARIES} ${PolyVoxUtil_LIBRARIES} ${BOOST_LIBRARIES})
ENDIF()

#IF (WIN32) #Windows
SET_TARGET_PROPERTIES(Thermite PROPERTIES COMPILE_DEFINITIONS "NOMINMAX") #Avoid Windows macros
#ENDIF (WIN32)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/plugins.cfg.in ${CMAKE_CURRENT_BINARY_DIR}/plugins.cfg)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/plugins_d.cfg.in ${CMAKE_CURRENT_BINARY_DIR}/plugins_d.cfg)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/plugins.cfg DESTINATION bin CONFIGURATIONS Release)	
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/plugins_d.cfg DESTINATION bin CONFIGURATIONS Debug)	

#Install the resulting binaries
INSTALL(TARGETS Thermite
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

#IF (WIN32) #Windows
#CONFIGURE_FILE(${OGRE_LIBRARY_DIRS}/OgreMain_d.dll ${CMAKE_CURRENT_BINARY_DIR}/debug/OgreMain_d.dll COPYONLY)
#CONFIGURE_FILE(${OGRE_LIBRARY_DIRS}/RenderSystem_GL_d.dll ${CMAKE_CURRENT_BINARY_DIR}/debug/RenderSystem_GL_d.dll COPYONLY)
#CONFIGURE_FILE(${OGRE_LIBRARY_DIRS}/RenderSystem_Direct3D9_d.dll ${CMAKE_CURRENT_BINARY_DIR}/debug/RenderSystem_Direct3D9_d.dll COPYONLY)

#CONFIGURE_FILE(${OGRE_LIBRARY_DIRS}/OgreMain.dll ${CMAKE_CURRENT_BINARY_DIR}/release/OgreMain.dll COPYONLY)
#CONFIGURE_FILE(${OGRE_LIBRARY_DIRS}/RenderSystem_GL.dll ${CMAKE_CURRENT_BINARY_DIR}/release/RenderSystem_GL.dll COPYONLY)
#CONFIGURE_FILE(${OGRE_LIBRARY_DIRS}/RenderSystem_Direct3D9.dll ${CMAKE_CURRENT_BINARY_DIR}/release/RenderSystem_Direct3D9.dll COPYONLY)
#ENDIF (WIN32)

#On Windows, it is necessary to copy all the dependancies .dll's to the binary folder
#Should be able to use INCLUDE(GetPrerequisites)
IF(WIN32)
	SET(OGRESDK $ENV{OGRE_HOME})
	SET(OGRESOURCE $ENV{OGRE_SRC})
	IF (OGRESDK)
		MESSAGE(STATUS "Using stuff from OGRE SDK")
		#This first bunch of install commands is called for release mode.
		INSTALL(FILES $ENV{OGRE_HOME}/bin/release/OgreMain.dll DESTINATION bin CONFIGURATIONS Release)

		INSTALL(FILES $ENV{OGRE_HOME}/bin/release/Cg.dll DESTINATION bin CONFIGURATIONS Release)

		INSTALL(FILES $ENV{OGRE_HOME}/bin/release/RenderSystem_GL.dll DESTINATION bin CONFIGURATIONS Release)
		INSTALL(FILES $ENV{OGRE_HOME}/bin/release/RenderSystem_Direct3D9.dll DESTINATION bin CONFIGURATIONS Release)
		INSTALL(FILES $ENV{OGRE_HOME}/bin/release/Plugin_CgProgramManager.dll DESTINATION bin CONFIGURATIONS Release)
		INSTALL(FILES $ENV{OGRE_HOME}/bin/release/Plugin_ParticleFX.dll DESTINATION bin CONFIGURATIONS Release)
		
		#This second bunch of install commands is called for debug mode.
		INSTALL(FILES $ENV{OGRE_HOME}/bin/debug/OgreMain_d.dll DESTINATION bin CONFIGURATIONS Debug)

		INSTALL(FILES $ENV{OGRE_HOME}/bin/release/Cg.dll DESTINATION bin CONFIGURATIONS Debug)

		INSTALL(FILES $ENV{OGRE_HOME}/bin/debug/RenderSystem_GL_d.dll DESTINATION bin CONFIGURATIONS Debug)
		INSTALL(FILES $ENV{OGRE_HOME}/bin/debug/RenderSystem_Direct3D9_d.dll DESTINATION bin CONFIGURATIONS Debug)
		INSTALL(FILES $ENV{OGRE_HOME}/bin/debug/Plugin_CgProgramManager_d.dll DESTINATION bin CONFIGURATIONS Debug)
		INSTALL(FILES $ENV{OGRE_HOME}/bin/debug/Plugin_ParticleFX_d.dll DESTINATION bin CONFIGURATIONS Debug)
	ENDIF (OGRESDK)
	IF (OGRESOURCE)
		MESSAGE(STATUS "Using stuff from OGRE Source")
		#This first bunch of install commands is called for release mode.
		INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Release/OgreMain.dll DESTINATION bin CONFIGURATIONS Release)

		INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Release/Cg.dll DESTINATION bin CONFIGURATIONS Release)

		INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Release/RenderSystem_GL.dll DESTINATION bin CONFIGURATIONS Release)
		INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Release/RenderSystem_Direct3D9.dll DESTINATION bin CONFIGURATIONS Release)
		INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Release/Plugin_CgProgramManager.dll DESTINATION bin CONFIGURATIONS Release)
		INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Release/Plugin_ParticleFX.dll DESTINATION bin CONFIGURATIONS Release)

		#This second bunch of install commands is called for debug mode.
		INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Debug/OgreMain_d.dll DESTINATION bin CONFIGURATIONS Debug)

		INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Release/Cg.dll DESTINATION bin CONFIGURATIONS Debug)

		INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Debug/RenderSystem_GL_d.dll DESTINATION bin CONFIGURATIONS Debug)
		INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Debug/RenderSystem_Direct3D9_d.dll DESTINATION bin CONFIGURATIONS Debug)
		INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Debug/Plugin_CgProgramManager_d.dll DESTINATION bin CONFIGURATIONS Debug)
		INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Debug/Plugin_ParticleFX_d.dll DESTINATION bin CONFIGURATIONS Debug)
	ENDIF (OGRESOURCE)
	
	#Install the PolyVox .dll's
	INSTALL(FILES $ENV{POLYVOX_HOME}/PolyVoxCore/bin/PolyVoxCore.dll DESTINATION bin)
	INSTALL(FILES $ENV{POLYVOX_HOME}/PolyVoxUtil/bin/PolyVoxUtil.dll DESTINATION bin)
	
	#And the debug files
	INSTALL(FILES $ENV{POLYVOX_HOME}/PolyVoxCore/lib/PolyVoxCore.pdb DESTINATION bin CONFIGURATIONS Debug)
	INSTALL(FILES $ENV{POLYVOX_HOME}/PolyVoxUtil/lib/PolyVoxUtil.pdb DESTINATION bin CONFIGURATIONS Debug)
	
	#Install the Qt .dll's
	INSTALL(FILES ${QT_BINARY_DIR}/QtCore4.dll DESTINATION bin CONFIGURATIONS Release)
	INSTALL(FILES ${QT_BINARY_DIR}/QtGui4.dll DESTINATION bin CONFIGURATIONS Release)
	INSTALL(FILES ${QT_BINARY_DIR}/QtSvg4.dll DESTINATION bin CONFIGURATIONS Release)
	INSTALL(FILES ${QT_BINARY_DIR}/QtXml4.dll DESTINATION bin CONFIGURATIONS Release)
	INSTALL(FILES ${QT_PLUGINS_DIR}/imageformats/qmng4.dll DESTINATION bin/qt-plugins/imageformats CONFIGURATIONS Release)
	INSTALL(FILES ${QT_PLUGINS_DIR}/imageformats/qsvg4.dll DESTINATION bin/qt-plugins/imageformats CONFIGURATIONS Release)
	
	INSTALL(FILES ${QT_BINARY_DIR}/QtCored4.dll DESTINATION bin CONFIGURATIONS Debug)
	INSTALL(FILES ${QT_BINARY_DIR}/QtGuid4.dll DESTINATION bin CONFIGURATIONS Debug)
	INSTALL(FILES ${QT_BINARY_DIR}/QtSvgd4.dll DESTINATION bin CONFIGURATIONS Debug)
	INSTALL(FILES ${QT_BINARY_DIR}/QtXmld4.dll DESTINATION bin CONFIGURATIONS Debug)
	INSTALL(FILES ${QT_PLUGINS_DIR}/imageformats/qmngd4.dll DESTINATION bin/qt-plugins/imageformats CONFIGURATIONS Debug)
	INSTALL(FILES ${QT_PLUGINS_DIR}/imageformats/qsvgd4.dll DESTINATION bin/qt-plugins/imageformats CONFIGURATIONS Debug)
	
	#Install the qt configuration file
	INSTALL(FILES qt.conf DESTINATION bin CONFIGURATIONS Release)	
	INSTALL(FILES qt.conf DESTINATION bin CONFIGURATIONS Debug)
	
	#Install the settings file
	INSTALL(FILES Settings.ini DESTINATION bin CONFIGURATIONS Release)	
	INSTALL(FILES Settings.ini DESTINATION bin CONFIGURATIONS Debug)	
	
ENDIF(WIN32)

#Lastly, we copy the core resources and set up the 'share' folder
INSTALL(DIRECTORY resources/Ogre/debug DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/resources/debug PATTERN "*.svn*" EXCLUDE) #this will exclude the .svn files
INSTALL(DIRECTORY DESTINATION ${CMAKE_INSTALL_PREFIX}/share/thermite/apps)