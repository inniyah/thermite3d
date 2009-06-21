CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

PROJECT(Thermite)

#Standard modules
#FIND_PACKAGE(Boost 1.34.1 COMPONENTS program_options filesystem REQUIRED)

#Custom modules
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
FIND_PACKAGE(Bullet REQUIRED)
FIND_PACKAGE(OGRE 1.6 REQUIRED)
FIND_PACKAGE(OGREBullet REQUIRED)
FIND_PACKAGE(PolyVox REQUIRED)
FIND_PACKAGE(QtOgre REQUIRED)

SET(SRC_FILES
	source/main.cpp
	source/CannonController.cpp
	source/DataStreamAdapter.cpp
	source/LoadingProgress.cpp
	source/LoadMapWidget.cpp
	source/MainMenu.cpp
	source/Map.cpp
	source/MapHandler.cpp
	source/MapRegion.cpp
	source/MultiThreadedSurfaceExtractor.cpp
	source/PhysicalEntity.cpp
	source/Shell.cpp
	source/SurfaceExtractorTaskData.cpp
	source/SurfaceExtractorThread.cpp
	source/SurfacePatchRenderable.cpp
	source/ThermiteGameLogic.cpp
	source/Utility.cpp
	source/VolumeManager.cpp
	source/VolumeResource.cpp
	source/VolumeSerializationProgressListenerImpl.cpp
	source/VolumeSerializer.cpp
)

SET(INC_FILES
	include/CannonController.h
	include/DataStreamAdapter.h
	include/LoadingProgress.h
	include/LoadMapWidget.h
	include/MainMenu.h
	include/Map.h
	include/MapHandler.h
	include/MapRegion.h
	include/MultiThreadedSurfaceExtractor.h
	include/PhysicalEntity.h
	include/Shell.h
	include/SurfaceExtractorTaskData.h
	include/SurfaceExtractorThread.h
	include/SurfacePatchRenderable.h
	include/ThermiteForwardDeclarations.h
	include/ThermiteGameLogic.h
	include/Utility.h
	include/VolumeManager.h
	include/VolumeResource.h
	include/VolumeSerializationProgressListenerImpl.h
	include/VolumeSerializer.h

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

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/include ${QTOGRE_INCLUDE_DIR} ${OGRE_INCLUDE_DIRS} ${POLYVOXCORE_INCLUDE_DIR} ${POLYVOXUTIL_INCLUDE_DIR} ${Boost_INCLUDE_DIRS} ${BULLET_INCLUDE_DIRS} ${OGREBULLET_INCLUDE_DIR})
LINK_DIRECTORIES(${OGRE_LIBRARY_DIRS} ${QTOGRE_LIBRARY_DIRS} ${POLYVOXCORE_LIB_DIR} ${POLYVOXUTIL_LIB_DIR} ${Boost_LIBRARY_DIRS} ${BULLET_LIBRARY_DIRS} ${OGREBULLET_LIB_DIR})
ADD_EXECUTABLE(Thermite WIN32 ${SRC_FILES}  ${MOC_SRCS} ${UI_SRCS} ${RESOURCE_SRCS})
TARGET_LINK_LIBRARIES(Thermite ${QT_LIBRARIES} ${OGRE_LIBRARIES} ${QTOGRE_LIBRARIES} ${POLYVOX_LIBRARIES} ${BOOST_LIBRARIES} ${OGREBULLET_LIBRARIES} ${BULLET_LIBRARIES})

#Install
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

#Should be able to use INCLUDE(GetPrerequisites)
IF(WIN32)
	SET(OGRESDK $ENV{OGRE_HOME})
	SET(OGRESOURCE $ENV{OGRE_SRC})
	IF (OGRESDK)
		MESSAGE(STATUS "Using stuff from OGRE SDK")
		#This first bunch of install commands is called for release mode.
		INSTALL(FILES $ENV{OGRE_HOME}/bin/release/OgreMain.dll DESTINATION bin CONFIGURATIONS Release)
		#INSTALL(FILES $ENV{OGRE_HOME}/bin/release/OIS.dll DESTINATION bin CONFIGURATIONS Release)

		#INSTALL(FILES $ENV{OGRE_HOME}/bin/release/CEGUIBase.dll DESTINATION bin CONFIGURATIONS Release)
		#INSTALL(FILES $ENV{OGRE_HOME}/bin/release/OgreGuiRenderer.dll DESTINATION bin CONFIGURATIONS Release)
		#INSTALL(FILES $ENV{OGRE_HOME}/bin/release/CEGUIExpatParser.dll DESTINATION bin CONFIGURATIONS Release)
		#INSTALL(FILES $ENV{OGRE_HOME}/bin/release/CEGUIFalagardWRBase.dll DESTINATION bin CONFIGURATIONS Release)

		INSTALL(FILES $ENV{OGRE_HOME}/bin/release/Cg.dll DESTINATION bin CONFIGURATIONS Release)

		INSTALL(FILES $ENV{OGRE_HOME}/bin/release/RenderSystem_GL.dll DESTINATION bin CONFIGURATIONS Release)
		INSTALL(FILES $ENV{OGRE_HOME}/bin/release/RenderSystem_Direct3D9.dll DESTINATION bin CONFIGURATIONS Release)
		INSTALL(FILES $ENV{OGRE_HOME}/bin/release/Plugin_CgProgramManager.dll DESTINATION bin CONFIGURATIONS Release)
		INSTALL(FILES $ENV{OGRE_HOME}/bin/release/Plugin_ParticleFX.dll DESTINATION bin CONFIGURATIONS Release)
		
		#This second bunch of install comands is called for debug mode.
		INSTALL(FILES $ENV{OGRE_HOME}/bin/debug/OgreMain_d.dll DESTINATION bin CONFIGURATIONS Debug)
		#INSTALL(FILES $ENV{OGRE_HOME}/bin/debug/OIS_d.dll DESTINATION bin CONFIGURATIONS Debug)

		#INSTALL(FILES $ENV{OGRE_HOME}/bin/debug/CEGUIBase_d.dll DESTINATION bin CONFIGURATIONS Debug)
		#INSTALL(FILES $ENV{OGRE_HOME}/bin/debug/OgreGuiRenderer_d.dll DESTINATION bin CONFIGURATIONS Debug)
		#INSTALL(FILES $ENV{OGRE_HOME}/bin/debug/CEGUIExpatParser_d.dll DESTINATION bin CONFIGURATIONS Debug)
		#INSTALL(FILES $ENV{OGRE_HOME}/bin/debug/CEGUIFalagardWRBase_d.dll DESTINATION bin CONFIGURATIONS Debug)

		INSTALL(FILES $ENV{OGRE_HOME}/bin/release/Cg.dll DESTINATION bin CONFIGURATIONS Debug)

		INSTALL(FILES $ENV{OGRE_HOME}/bin/debug/RenderSystem_GL_d.dll DESTINATION bin CONFIGURATIONS Debug)
		INSTALL(FILES $ENV{OGRE_HOME}/bin/debug/RenderSystem_Direct3D9_d.dll DESTINATION bin CONFIGURATIONS Debug)
		INSTALL(FILES $ENV{OGRE_HOME}/bin/debug/Plugin_CgProgramManager_d.dll DESTINATION bin CONFIGURATIONS Debug)
		INSTALL(FILES $ENV{OGRE_HOME}/bin/debug/Plugin_ParticleFX_d.dll DESTINATION bin CONFIGURATIONS Debug)
	ENDIF (OGRESDK)
	IF (OGRESOURCE)
		MESSAGE(STATUS "Using stuff from OGRE Source")
		INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Release/OgreMain.dll DESTINATION bin CONFIGURATIONS Release)
		#INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Release/OIS.dll DESTINATION bin CONFIGURATIONS Release)

		#INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Release/CEGUIBase.dll DESTINATION bin CONFIGURATIONS Release)
		#INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Release/OgreGuiRenderer.dll DESTINATION bin CONFIGURATIONS Release)
		#INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Release/CEGUIExpatParser.dll DESTINATION bin CONFIGURATIONS Release)
		#INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Release/CEGUIFalagardWRBase.dll DESTINATION bin CONFIGURATIONS Release)

		INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Release/Cg.dll DESTINATION bin CONFIGURATIONS Release)

		INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Release/RenderSystem_GL.dll DESTINATION bin CONFIGURATIONS Release)
		INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Release/RenderSystem_Direct3D9.dll DESTINATION bin CONFIGURATIONS Release)
		INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Release/Plugin_CgProgramManager.dll DESTINATION bin CONFIGURATIONS Release)
		INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Release/Plugin_ParticleFX.dll DESTINATION bin CONFIGURATIONS Release)

		#This second bunch of install comands is called for debug mode.
		INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Debug/OgreMain_d.dll DESTINATION bin CONFIGURATIONS Debug)
		#INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Debug/OIS_d.dll DESTINATION bin CONFIGURATIONS Debug)

		#INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Debug/CEGUIBase_d.dll DESTINATION bin CONFIGURATIONS Debug)
		#INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Debug/OgreGuiRenderer_d.dll DESTINATION bin CONFIGURATIONS Debug)
		#INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Debug/CEGUIExpatParser_d.dll DESTINATION bin CONFIGURATIONS Debug)
		#INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Debug/CEGUIFalagardWRBase_d.dll DESTINATION bin CONFIGURATIONS Debug)

		INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Release/Cg.dll DESTINATION bin CONFIGURATIONS Debug)

		INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Debug/RenderSystem_GL_d.dll DESTINATION bin CONFIGURATIONS Debug)
		INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Debug/RenderSystem_Direct3D9_d.dll DESTINATION bin CONFIGURATIONS Debug)
		INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Debug/Plugin_CgProgramManager_d.dll DESTINATION bin CONFIGURATIONS Debug)
		INSTALL(FILES $ENV{OGRE_SRC}/Samples/Common/bin/Debug/Plugin_ParticleFX_d.dll DESTINATION bin CONFIGURATIONS Debug)
	ENDIF (OGRESOURCE)
	
	#Install the PolyVox .dll's
	INSTALL(FILES $ENV{POLYVOX_HOME}/PolyVoxCore/bin/PolyVoxCore.dll DESTINATION bin CONFIGURATIONS Release)
	INSTALL(FILES $ENV{POLYVOX_HOME}/PolyVoxUtil/bin/PolyVoxUtil.dll DESTINATION bin CONFIGURATIONS Release)
	
	INSTALL(FILES $ENV{POLYVOX_HOME}/PolyVoxCore/bin/PolyVoxCore_d.dll DESTINATION bin CONFIGURATIONS Debug)
	INSTALL(FILES $ENV{POLYVOX_HOME}/PolyVoxUtil/bin/PolyVoxUtil_d.dll DESTINATION bin CONFIGURATIONS Debug)
	
	#Install the settings file
	INSTALL(FILES Settings.ini DESTINATION bin CONFIGURATIONS Release)	
	INSTALL(FILES Settings.ini DESTINATION bin CONFIGURATIONS Debug)

	
	
ENDIF(WIN32)

#INSTALL(DIRECTORY resources DESTINATION ${CMAKE_INSTALL_PREFIX}/bin PATTERN "*.svn*" EXCLUDE) #this will exclude the .svn files
#INSTALL(DIRECTORY media DESTINATION ${CMAKE_INSTALL_PREFIX}/share/thermite PATTERN "*.svn*" EXCLUDE) #this will exclude the .svn files
INSTALL(DIRECTORY resources/Ogre DESTINATION ${CMAKE_INSTALL_PREFIX}/share/thermite PATTERN "*.svn*" EXCLUDE) #this will exclude the .svn files
#INSTALL(FILES resource/resources.cfg DESTINATION ${CMAKE_INSTALL_PREFIX}/share/thermite)