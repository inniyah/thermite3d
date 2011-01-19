/*******************************************************************************
Copyright (c) 2005-2009 David Williams

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution. 	
*******************************************************************************/

#ifndef THERMITEGAMELOGIC_H_
#define THERMITEGAMELOGIC_H_

#include "AnyOption.h"
#include "GameLogic.h"
#include "Serialization.h"

#include "Camera.h"
#include "Entity.h"
#include "Globals.h"
#include "Light.h"
#include "ScriptEditorWidget.h"

#include <QtScript>
#include <QScriptEngineDebugger>

#include "scriptable/Volume.h"
#include "ThermiteForwardDeclarations.h"

#include "QtOgreForwardDeclarations.h"

#include <OgrePrerequisites.h>
#include <OgreTexture.h>

#include <QHash>

#include <QTime>

#include <QMovie>
#include <QLabel>
#include <QObject>

#include <list>
#include <queue>

namespace Thermite
{
	class MainMenu;

	class ThermiteGameLogic : public QObject, public QtOgre::GameLogic
	{
		Q_OBJECT
	public:
		ThermiteGameLogic(void);

		void setupScripting(void);

		void initialise(void);
		void update(void);
		void shutdown(void);

		QtOgre::Log* thermiteLog(void);

		void reloadShaders(void);

		void onKeyPress(QKeyEvent* event);
		void onKeyRelease(QKeyEvent* event);

		void onMouseMove(QMouseEvent* event);
		void onMousePress(QMouseEvent* event);
		void onMouseRelease(QMouseEvent* event);

		void onWheel(QWheelEvent* event);

		void initScriptEngine(void);
		void initScriptEnvironment(void);

	public:
		void addResourceDirectory(const QString& directoryName);

		void createAxis(void);

	public:		

	public slots:
		void uploadSurfaceMesh(const PolyVox::SurfaceMesh<PolyVox::PositionMaterial>& mesh, PolyVox::Region region, Volume& volume);		
		void addSurfacePatchRenderable(std::string materialName, PolyVox::SurfaceMesh<PolyVox::PositionMaterial>& mesh, PolyVox::Region region);

		void uploadSurfaceMesh(const PolyVox::SurfaceMesh<PolyVox::PositionMaterialNormal>& mesh, PolyVox::Region region, Volume& volume);		
		void addSurfacePatchRenderable(std::string materialName, PolyVox::SurfaceMesh<PolyVox::PositionMaterialNormal>& mesh, PolyVox::Region region);

		polyvox_shared_ptr< PolyVox::SurfaceMesh<PolyVox::PositionMaterialNormal> > extractSubset(std::set<uint8_t> setMaterials, PolyVox::SurfaceMesh<PolyVox::PositionMaterialNormal>& inputMesh);

		bool loadApp(const QString& appName);
		void unloadApp(void);
		QWidget* loadUIFile(const QString& filename);
		void include(QString path);

		//Don't like having these functions here - really they should be inside camera or something. But they are
		//using Ogre methods which aren't available in the scriptable classes. Eventually they should be rewritten
		//without the utility classes. Also, there are two seperate functions because I couldn't pass QVector3D
		//by reference. Need to look at registering this type...
		QVector3D getPickingRayOrigin(int x, int y);
		QVector3D getPickingRayDir(int x, int y);

	private slots:
		void startScriptingEngine(void);
		void stopScriptingEngine(void);

		void playStartupMovie(void);
		void showLastMovieFrame(void);
		void deleteMovie(void);
		
	private:

		//Deletes all children (both nodes and attached objects) but not the node itself.
		void deleteSceneNodeChildren(Ogre::SceneNode* sceneNode);
		void exposeFunction(const QString& functionName, const QString& params);

		//Scene representation
		Camera* mCamera;
		SkyBox* mSkyBox;
		PolyVox::Array<3, uint32_t> mVolLastUploadedTimeStamps;
		PolyVox::Array<3, uint32_t> mVolLightingLastUploadedTimeStamps;
		QList<Object*> mObjectList;
		uint16_t mCachedVolumeWidthInRegions;
		uint16_t mCachedVolumeHeightInRegions;
		uint16_t mCachedVolumeDepthInRegions;

		uint16_t mCachedVolumeWidthInLightRegions;
		uint16_t mCachedVolumeHeightInLightRegions;
		uint16_t mCachedVolumeDepthInLightRegions;

		Ogre::TexturePtr mAmbientOcclusionVolumeTexture;

		//Ogre's scene representation
		Ogre::SceneNode* mVolumeSceneNode;
		PolyVox::Array<3, Ogre::SceneNode*> m_volOgreSceneNodes;
		Ogre::SceneNode* mCameraSceneNode;
		Ogre::Camera* mOgreCamera;
		Ogre::Viewport* mMainViewport;
		Ogre::SceneManager* mOgreSceneManager;
		Ogre::SceneNode* m_axisNode;

		//Input
		Keyboard* keyboard;
		Mouse* mouse;

		//Scripting support
		QScriptEngine* scriptEngine;
		ScriptEditorWidget* m_pScriptEditorWidget;
		bool m_bRunScript;
		QString mInitialiseScript;

		//User interface
		Console* mConsole;
		MainMenu* mMainMenu;
		QMovie* m_pThermiteLogoMovie;
		QLabel* m_pThermiteLogoLabel;

		//Other
		bool mFirstFind;
		QtOgre::Log* mThermiteLog;
		AnyOption m_commandLineArgs;
		QString mCurrentAppName;
	};
}

#endif /*DEMOGAMELOGIC_H_*/
