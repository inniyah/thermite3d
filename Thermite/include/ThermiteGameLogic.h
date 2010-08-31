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
#include "SurfaceExtractorTaskData.h"
#include "VolumeChangeTracker.h"

#include "Camera.h"
#include "Entity.h"
#include "Globals.h"
#include "Keyboard.h"
#include "Light.h"
#include "Mouse.h"
#include "ObjectStore.h"
#include "ScriptEditorWidget.h"

#include <QtScript>
#include <QScriptEngineDebugger>

#include "Scriptable/Volume.h"
#include "PhysicalEntity.h"
#include "ThermiteForwardDeclarations.h"

#include "QtOgreForwardDeclarations.h"

#include <OgrePrerequisites.h>

#include <QHash>

#include <QTime>

#include <QMovie>
#include <QLabel>
#include <QObject>

#include <list>
#include <queue>

namespace Thermite
{
	enum KeyStates
	{
		KS_RELEASED,
		KS_PRESSED
	};

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

		void createAxis(unsigned int uWidth, unsigned int uHeight, unsigned int uDepth);

		
		QTime* mTime;

		int mLastFrameTime;
		int mCurrentTime;
		float mTimeElapsedInSeconds;

		unsigned int mCurrentFrameNumber;

		

		MoviePlayer* mMoviePlayer;

		Ogre::Camera* mOgreCamera;
		Ogre::Viewport* mMainViewport;
		Ogre::SceneManager* m_pOgreSceneManager;
		QtOgre::Log* mThermiteLog;

		//Thermite stuff
		std::queue<PhysicalObject*> m_queueObjects;

		QMovie* m_pThermiteLogoMovie;
		QLabel* m_pThermiteLogoLabel;

		Ogre::SceneNode* m_axisNode;

		AnyOption m_commandLineArgs;

	public:		

	public slots:
		void uploadSurfaceMesh(const PolyVox::SurfaceMesh& mesh, PolyVox::Region region, Volume& volume);		
		void addSurfacePatchRenderable(std::string materialName, PolyVox::SurfaceMesh& mesh, PolyVox::Region region);

	private slots:
		void startScriptingEngine(void);
		void stopScriptingEngine(void);

		void playStartupMovie(void);
		void showLastMovieFrame(void);
		void deleteMovie(void);
		
	public:

		

#ifdef ENABLE_BULLET_PHYSICS
		OgreBulletDynamics::DynamicsWorld *m_pOgreBulletWorld;
#endif //ENABLE_BULLET_PHYSICS	

		PolyVox::Volume<uint32_t>* m_volLastUploadedTimeStamps;
		PolyVox::Volume<Ogre::SceneNode*>* m_volOgreSceneNodes;

		bool mFirstFind;

		MainMenu* mMainMenu;

		Keyboard keyboard;
		Mouse* mouse;

		//Scripting
		QScriptEngine* scriptEngine;

		Camera* camera;

		ScriptEditorWidget* m_pScriptEditorWidget;

		bool m_bRunScript;

		QHash<QString, Light*> m_Lights;

		ObjectStore mObjectStore;

		QString mInitialiseScript;

		Globals* mGlobals;
	};
}

#endif /*DEMOGAMELOGIC_H_*/