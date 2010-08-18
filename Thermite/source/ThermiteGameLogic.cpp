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

#include "ThermiteGameLogic.h"

#include "TaskProcessorThread.h"
#include "SurfaceMeshDecimationTask.h"
#include "SurfaceMeshExtractionTask.h"
#include "SurfaceExtractorTaskData.h"
#include "SurfacePatchRenderable.h"
#include "MapManager.h"
#include "MaterialDensityPair.h"
#include "VolumeManager.h"

#include "Application.h"
#include "LogManager.h"

#include "MainMenu.h"
#include "LoadMapWidget.h"

#include <OgreEntity.h>
#include <OgreRenderWindow.h>
#include <OgreResourceGroupManager.h>
#include <OgreRoot.h>

#include <QDirIterator>
#include <QMovie>
#include <QMutex>
#include <QSettings>
#include <QThreadPool>
#include <QWaitCondition>

#ifdef ENABLE_BULLET_PHYSICS
	#include "OgreBulletDynamicsWorld.h"
	#include "Shapes/OgreBulletCollisionsBoxShape.h"
#endif //ENABLE_BULLET_PHYSICS

#ifdef ENABLE_BULLET_PHYSICS
using namespace OgreBulletDynamics;
using namespace OgreBulletCollisions;
#endif //ENABLE_BULLET_PHYSICS

using namespace QtOgre;

using namespace std;

using namespace PolyVox;

namespace Thermite
{
	ThermiteGameLogic::ThermiteGameLogic(void)
		:GameLogic()
		,mCurrentFrameNumber(0)
		,mMap(0)
		,m_pOgreSceneManager(0)
		,m_bRunScript(true)
		,scriptEngine(0)
		,camera(0)
		,m_pScriptEditorWidget(0)
		,mOgreCamera(0)
		,hasVolume(false)
	{
		qRegisterMetaType<SurfaceExtractorTaskData>("SurfaceExtractorTaskData");

		mouse = new Mouse(this);
		camera = new Camera(this);
	}

	void ThermiteGameLogic::setupScripting(void)
	{
		initScriptEngine();	 

		initScriptEnvironment();

		QString script
		(
		"print('In initialise');"
		"test = new QPoint();"
		"print(test);"
		"print('Done initialise');"
		);

		QScriptValue result = scriptEngine->evaluate(script);
		if (scriptEngine->hasUncaughtException())
		{
			int line = scriptEngine->uncaughtExceptionLineNumber();
			qCritical() << "uncaught exception at line" << line << ":" << result.toString();
		}

		result = scriptEngine->evaluate(script);
		if (scriptEngine->hasUncaughtException())
		{
			int line = scriptEngine->uncaughtExceptionLineNumber();
			qCritical() << "uncaught exception at line" << line << ":" << result.toString();
		}

		result = scriptEngine->evaluate(script);
		if (scriptEngine->hasUncaughtException())
		{
			int line = scriptEngine->uncaughtExceptionLineNumber();
			qCritical() << "uncaught exception at line" << line << ":" << result.toString();
		}

		mInitialiseScript =
		"print('QtScript Initialisation Begin');"

		"var redLight = new Light();"
		"redLight.position = new QVector3D(100,100,100);"
		"redLight.colour = new QColor(255,0,0);"
		"objectStore.setObject('RedLight', redLight);"

		"var greenLight = new Light();"
		"greenLight.position = new QVector3D(100,100,100);"
		"greenLight.colour = new QColor(0,255,0);"
		"objectStore.setObject('GreenLight', greenLight);"

		"var blueLight = new Light();"
		"blueLight.position = new QVector3D(100,100,100);"
		"blueLight.colour = new QColor(0,0,255);"
		"objectStore.setObject('BlueLight', blueLight);"

		"var robot = new Entity();"
		"robot.position = new QVector3D(3,-1,0);"
		"robot.size = new QVector3D(1.0, 1.0, 1.0);"
		"robot.meshName = 'robot.mesh';"
		"robot.animated = true;"
		"robot.loopAnimation = true;"
		"robot.animationName = 'Walk';"
		"objectStore.setObject('Robot', robot);"

		"var sphere = new Entity();"
		"sphere.meshName = 'sphere.mesh';"
		"objectStore.setObject('Sphere', sphere);"

		"var map = new Map();"
		"objectStore.setObject('Map', map);"

		"print('QtScript Initialisation End');";

		result = scriptEngine->evaluate(mInitialiseScript);
		if (scriptEngine->hasUncaughtException())
		{
			int line = scriptEngine->uncaughtExceptionLineNumber();
			qCritical() << "uncaught exception at line" << line << ":" << result.toString();
		}
	}

	void ThermiteGameLogic::initialise(void)
	{	
		//Parse the command line options
		m_commandLineArgs.setOption("appname");
		m_commandLineArgs.processCommandArgs(qApp->argc(), qApp->argv());

		//Set up the logging system , to hopefully record any problems.
		mThermiteLog = mApplication->createLog("Thermite");
		mThermiteLog->logMessage("Initialising Thermite3D Game Engine", LL_INFO);

		//Set the main window icon
		QIcon mainWindowIcon(QPixmap(QString::fromUtf8(":/images/thermite_logo.svg")));
		qApp->mainWidget()->setWindowIcon(mainWindowIcon);

		//We have to create a scene manager and viewport here so that the screen
		//can be cleared to black befre the Thermite logo animation is played.
		m_pOgreSceneManager = new Ogre::DefaultSceneManager("DummySceneManager");
		mOgreCamera = m_pOgreSceneManager->createCamera("DummyCamera");

		mOgreCamera->setPosition(128, 128, 128);
		mOgreCamera->lookAt(128, 0, 128);
		mOgreCamera->setFOVy(Ogre::Radian(1.0));
		mOgreCamera->setNearClipDistance(1.0);
		mOgreCamera->setFarClipDistance(1000);

		//m_pOgreSceneManager->getRootSceneNode()->attachObject(mOgreCamera);
		mMainViewport = mApplication->ogreRenderWindow()->addViewport(mOgreCamera);
		mMainViewport->setBackgroundColour(Ogre::ColourValue::Black);

		//Set up and start the thermite logo animation. This plays while we initialise.
		m_pThermiteLogoMovie = new QMovie(QString::fromUtf8(":/animations/thermite_logo.mng"));
		m_pThermiteLogoLabel = new QLabel(qApp->mainWidget(), Qt::FramelessWindowHint | Qt::Tool);
		/*m_pThermiteLogoLabel->setMovie(m_pThermiteLogoMovie);
		m_pThermiteLogoMovie->jumpToFrame(0);
		m_pThermiteLogoLabel->resize(m_pThermiteLogoMovie->currentImage().size());
		m_pThermiteLogoLabel->show();
		m_pThermiteLogoMovie->start();*/

		//Initialise all resources
		addResourceDirectory("./resources/");
		char* strAppName = m_commandLineArgs.getValue("appname");
		if(strAppName != 0)
		{
			addResourceDirectory(QString("../share/thermite/apps/") + QString::fromAscii(strAppName));
		}
		Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

		//Used to display loading/extraction progress
		m_loadingProgress = new LoadingProgress(qApp->mainWidget(), Qt::Tool);
		Application::centerWidget(m_loadingProgress, qApp->mainWidget());

		//Set the frame sate to be as high as possible
		mApplication->setAutoUpdateInterval(0);

		//Some Ogre related stuff we need to set up
		Ogre::Root::getSingletonPtr()->addMovableObjectFactory(new SurfacePatchRenderableFactory);
		VolumeManager* vm = new VolumeManager;
		vm->m_pProgressListener = new VolumeSerializationProgressListenerImpl(this);

		MapManager* mm = new MapManager;

		mTime = new QTime;
		mTime->start();

		mCurrentFrameNumber = 0;

		m_backgroundThread = new TaskProcessorThread;
		m_backgroundThread->setPriority(QThread::LowestPriority);
		m_backgroundThread->start();

		//Slightly hacky way of sleeping while the logo animation plays.
		//FIXME - Think about using QMovie::finished() signal instead.
		/*QWaitCondition sleep;
		SignalableMutex mutex;
		mutex.lock();
		sleep.wait(&mutex, 2000);*/

		/*m_pThermiteLogoMovie->stop(); //Just incase we didn't stop already.
		QPixmap lastFrameOfMovie = m_pThermiteLogoMovie->currentPixmap();
		m_pThermiteLogoLabel->setPixmap(lastFrameOfMovie);
		delete m_pThermiteLogoMovie;*/

		//Application stuff
		//Set up various GUI components...
		//The load map widget
		LoadMapWidget* wgtLoadMap = new LoadMapWidget(this, qApp->mainWidget(), Qt::Tool);
		Application::centerWidget(wgtLoadMap, qApp->mainWidget());

		//The main menu
		mMainMenu = new MainMenu(qApp->mainWidget(), Qt::Tool);
		Application::centerWidget(mMainMenu, qApp->mainWidget());
		QObject::connect(mMainMenu, SIGNAL(resumeClicked(void)), mMainMenu, SLOT(hide(void)));
		QObject::connect(mMainMenu, SIGNAL(quitClicked(void)), qApp->mainWidget(), SLOT(close(void)));
		QObject::connect(mMainMenu, SIGNAL(settingsClicked(void)), mApplication, SLOT(showSettingsDialog(void)));
		QObject::connect(mMainMenu, SIGNAL(viewLogsClicked(void)), mApplication, SLOT(showLogManager(void)));
		QObject::connect(mMainMenu, SIGNAL(loadClicked(void)), wgtLoadMap, SLOT(show(void)));
		mMainMenu->show();

		//Show the main menu after the animation has finished
		//QTimer::singleShot(2000, mMainMenu, SLOT(show()));

		mCameraSpeed = 50.0;
		mCameraRotationalSpeed = 0.1;	

		qApp->mainWidget()->setMouseTracking(true);

		m_pScriptEditorWidget = new ScriptEditorWidget(qApp->mainWidget());
		m_pScriptEditorWidget->show();		

		QObject::connect(m_pScriptEditorWidget, SIGNAL(start(void)), this, SLOT(startScriptingEngine(void)));
		QObject::connect(m_pScriptEditorWidget, SIGNAL(stop(void)), this, SLOT(stopScriptingEngine(void)));

		m_pOgreSceneManager->setSkyBox(true, "SkyBox", 5000, true);
		mOgreCamera->setFOVy(Ogre::Radian(1.0f));

		//test();
	}

	void ThermiteGameLogic::update(void)
	{
		/*if(mMap == 0)
			return;*/

		mLastFrameTime = mCurrentTime;
		mCurrentTime = mTime->elapsed();

		mTimeElapsedInSeconds = (mCurrentTime - mLastFrameTime) / 1000.0f;

		mGlobals->setPreviousFrameTime(mGlobals->getCurrentFrameTime());
		mGlobals->setCurrentFrameTime(mCurrentTime);

		//The fun stuff!
		updatePolyVoxGeometry();
		
#ifdef ENABLE_BULLET_PHYSICS
		if((qApp->settings()->value("Physics/SimulatePhysics", false).toBool()) && (bLoadComplete))
		{
			m_pOgreBulletWorld->stepSimulation(mTimeElapsedInSeconds, 10);
		}
#endif //ENABLE_BULLET_PHYSICS

		++mCurrentFrameNumber;

		float distance = mCameraSpeed * mTimeElapsedInSeconds;

		if(m_bRunScript)
		{
			QScriptValue result = scriptEngine->evaluate(m_pScriptEditorWidget->getScriptCode());
			if (scriptEngine->hasUncaughtException())
			{
				int line = scriptEngine->uncaughtExceptionLineNumber();
				qCritical() << "uncaught exception at line" << line << ":" << result.toString();
			}
		}

		if(m_pOgreSceneManager)
		{
			m_pOgreSceneManager->destroyAllLights();
			QHashIterator<QString, QObject*> objectIter(mObjectStore);
			while(objectIter.hasNext())
			{
				objectIter.next();
				QObject* pObj = objectIter.value();

				Light* light = dynamic_cast<Light*>(pObj);
				if(light)
				{
					Ogre::Light* ogreLight = m_pOgreSceneManager->createLight(objectIter.key().toStdString());
					ogreLight->setType(Ogre::Light::LT_POINT);

					QVector3D pos = light->position();
					ogreLight->setPosition(Ogre::Vector3(pos.x(), pos.y(), pos.z()));

					QColor col = light->getColour();
					ogreLight->setDiffuseColour(col.redF(), col.greenF(), col.blueF());
				}

				Entity* entity = dynamic_cast<Entity*>(pObj);
				if(entity)
				{
					Ogre::Entity* ogreEntity;
					Ogre::SceneNode* sceneNode;

					if(m_pOgreSceneManager->hasEntity(objectIter.key().toStdString()))
					{
						ogreEntity = m_pOgreSceneManager->getEntity(objectIter.key().toStdString());
						sceneNode = dynamic_cast<Ogre::SceneNode*>(ogreEntity->getParentNode());
					}
					else
					{
						sceneNode = m_pOgreSceneManager->getRootSceneNode()->createChildSceneNode();
						ogreEntity = m_pOgreSceneManager->createEntity(objectIter.key().toStdString(), entity->meshName().toStdString());
						sceneNode->attachObject(ogreEntity);
					}

					QVector3D pos = entity->position();
					sceneNode->setPosition(Ogre::Vector3(pos.x(), pos.y(), pos.z()));

					QQuaternion orientation = entity->orientation();
					sceneNode->setOrientation(Ogre::Quaternion(orientation.scalar(), orientation.x(), orientation.y(), orientation.z()));

					QVector3D scale = entity->size();
					sceneNode->setScale(Ogre::Vector3(scale.x(), scale.y(), scale.z()));

					//Animation
					Ogre::AnimationStateSet* animationStateSet = ogreEntity->getAllAnimationStates();		
					if(animationStateSet && animationStateSet->hasAnimationState(entity->animationName().toStdString()))
					{
						Ogre::AnimationState* animationState = animationStateSet->getAnimationState(entity->animationName().toStdString());
						animationState->setEnabled(entity->animated());
						animationState->setLoop(entity->loopAnimation());
					}
				}
				Map* map = dynamic_cast<Map*>(pObj);
				if(map)
				{
					if(!hasVolume)
					{
						mMap = map;

						mMap->volumeResource = VolumeManager::getSingletonPtr()->load("castle.volume", "General");
						mMap->m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(1);
						mMap->m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(2);
						mMap->m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(3);
						mMap->m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(4);
						mMap->m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(5);
						mMap->m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(6);
						mMap->m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(7);
						mMap->m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(8);

						mMap->initialise();

						loadMap("sdfdsf");
						hasVolume = true;
					}
				}
			}
		}

		if(mOgreCamera)
		{
			mOgreCamera->setPosition(Ogre::Vector3(camera->position().x(), camera->position().y(), camera->position().z()));
			mOgreCamera->setOrientation(Ogre::Quaternion(camera->orientation().scalar(), camera->orientation().x(), camera->orientation().y(), camera->orientation().z()));
			mOgreCamera->setFOVy(Ogre::Radian(camera->fieldOfView()));
		}

		mouse->setPreviousPosition(mouse->position());
		mouse->resetWheelDelta();
	}

	void ThermiteGameLogic::onKeyPress(QKeyEvent* event)
	{
		keyboard.press(event->key());

		if(event->key() == Qt::Key_F5)
		{
			reloadShaders();
		}

		if(event->key() == Qt::Key_Escape)
		{
			mMainMenu->show();
		}
	}

	void ThermiteGameLogic::onKeyRelease(QKeyEvent* event)
	{
		keyboard.release(event->key());
	}

	void ThermiteGameLogic::onMousePress(QMouseEvent* event)
	{
		mouse->press(event->button());
	
		//Update the mouse position as well or we get 'jumps'
		mouse->setPosition(event->pos());
		mouse->setPreviousPosition(mouse->position());
	}

	void ThermiteGameLogic::onMouseRelease(QMouseEvent* event)
	{
		mouse->release(event->button());
	}

	void ThermiteGameLogic::onMouseMove(QMouseEvent* event)
	{
		//mCurrentMousePos = event->pos();
		mouse->setPosition(event->pos());
	}

	void ThermiteGameLogic::onWheel(QWheelEvent* event)
	{
		//mCurrentWheelPos += event->delta();
		mouse->modifyWheelDelta(event->delta());
	}

	void ThermiteGameLogic::onLoadMapClicked(QString strMapName)
	{
		mMainMenu->hide();
		//Temporary hack until loading new map is fixed...
		mMainMenu->disableLoadButton();

		loadMap(strMapName);

		mApplication->showFPSCounter();
	}

	void ThermiteGameLogic::shutdown(void)
	{
		Ogre::Root::getSingleton().destroySceneManager(m_pOgreSceneManager);
	}

	Log* ThermiteGameLogic::thermiteLog(void)
	{
		return mThermiteLog;
	}

	void ThermiteGameLogic::addResourceDirectory(const QString& directoryName)
	{
		QDir appDir(directoryName);
		if(appDir.exists())
		{
			//Add the directory to Ogre
			Ogre::ResourceGroupManager::getSingleton().addResourceLocation(directoryName.toStdString(), "FileSystem");

			//Add the subdirectories to Ogre
			QDirIterator it(directoryName, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
			while (it.hasNext())
			{
				Ogre::ResourceGroupManager::getSingleton().addResourceLocation(it.next().toStdString(), "FileSystem");
			}
		}
	}

	void ThermiteGameLogic::loadMap(QString strMapName)
	{
		bLoadComplete = false;
		m_pThermiteLogoLabel->hide();

		m_loadingProgress->show();

		m_iNoProcessed = 0;
		m_iNoSubmitted = 0;

		//mMap = new Map;
		mMap->m_pOgreSceneManager = m_pOgreSceneManager;
		

#ifdef ENABLE_BULLET_PHYSICS
		m_pOgreBulletWorld = mMap->m_pOgreBulletWorld;
#endif //ENABLE_BULLET_PHYSICS

		

		//mMap = mapResource->m_pMap;

		initialisePhysics();

		//if(qApp->settings()->value("Debug/ShowVolumeAxes", false).toBool())
		{
			createAxis(mMap->volumeResource->getVolume()->getWidth(), mMap->volumeResource->getVolume()->getHeight(), mMap->volumeResource->getVolume()->getDepth());
		}

		if(qApp->settings()->value("Shadows/EnableShadows", false).toBool())
		{
			m_pOgreSceneManager->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED);
			//m_pOgreSceneManager->setShadowFarDistance(1000.0f);
			m_pOgreSceneManager->setShadowTextureSelfShadow(true);
			m_pOgreSceneManager->setShadowTextureCasterMaterial("ShadowMapCasterMaterial");
			m_pOgreSceneManager->setShadowTexturePixelFormat(Ogre::PF_FLOAT32_R);
			m_pOgreSceneManager->setShadowCasterRenderBackFaces(true);
			m_pOgreSceneManager->setShadowTextureSize(qApp->settings()->value("Shadows/ShadowMapSize", 1024).toInt());
		}	


		//We've loaded a scene so let's free some memory by deleting the movie.
		//Later on we should handle this properly by replacing it with it's last
		//frame once it has finished playing.
		if(m_pThermiteLogoLabel != 0)
		{
			delete m_pThermiteLogoLabel;
			m_pThermiteLogoLabel = 0;
		}
		if(m_pThermiteLogoMovie != 0)
		{
			delete m_pThermiteLogoMovie;
			m_pThermiteLogoMovie = 0;
		}
	}

	void ThermiteGameLogic::setVolumeLoadProgress(float fProgress)
	{
		mThermiteLog->logMessage("loading...", LL_INFO);
		m_loadingProgress->setLoadingDataPercentageDone(fProgress*100);
	}

	void ThermiteGameLogic::createAxis(unsigned int uWidth, unsigned int uHeight, unsigned int uDepth)
	{
		float fWidth = static_cast<float>(uWidth);
		float fHeight = static_cast<float>(uHeight);
		float fDepth = static_cast<float>(uDepth);
		float fHalfWidth = fWidth/2.0;
		float fHalfHeight = fHeight/2.0;
		float fHalfDepth = fDepth/2.0;

		float fOriginSize = 4.0f;	
		Ogre::Vector3 vecToUnitCube(0.01,0.01,0.01);

		//Create the main node for the axes
		m_axisNode = m_pOgreSceneManager->getRootSceneNode()->createChildSceneNode();

		//Create sphere representing origin
		Ogre::SceneNode* originNode = m_axisNode->createChildSceneNode();
		Ogre::Entity *originSphereEntity = m_pOgreSceneManager->createEntity( "Origin Sphere", Ogre::SceneManager::PT_CUBE );
		originSphereEntity->setMaterialName("OriginMaterial");
		originNode->attachObject(originSphereEntity);
		originNode->scale(vecToUnitCube);
		originNode->scale(fOriginSize,fOriginSize,fOriginSize);

		//Create x-axis
		Ogre::SceneNode *xAxisCylinderNode = m_axisNode->createChildSceneNode();
		Ogre::Entity *xAxisCylinderEntity = m_pOgreSceneManager->createEntity( "X Axis", Ogre::SceneManager::PT_CUBE );
		xAxisCylinderEntity->setMaterialName("RedMaterial");
		xAxisCylinderNode->attachObject(xAxisCylinderEntity);	
		xAxisCylinderNode->scale(vecToUnitCube);
		xAxisCylinderNode->scale(Ogre::Vector3(fWidth,1.0,1.0));
		xAxisCylinderNode->translate(Ogre::Vector3(fHalfWidth,0.0,0.0));

		//Create y-axis
		Ogre::SceneNode *yAxisCylinderNode = m_axisNode->createChildSceneNode();
		Ogre::Entity *yAxisCylinderEntity = m_pOgreSceneManager->createEntity( "Y Axis", Ogre::SceneManager::PT_CUBE );
		yAxisCylinderEntity->setMaterialName("GreenMaterial");
		yAxisCylinderNode->attachObject(yAxisCylinderEntity);		
		yAxisCylinderNode->scale(vecToUnitCube);
		yAxisCylinderNode->scale(Ogre::Vector3(1.0,fHeight,1.0));
		yAxisCylinderNode->translate(Ogre::Vector3(0.0,fHalfHeight,0.0));

		//Create z-axis
		Ogre::SceneNode *zAxisCylinderNode = m_axisNode->createChildSceneNode();
		Ogre::Entity *zAxisCylinderEntity = m_pOgreSceneManager->createEntity( "Z Axis", Ogre::SceneManager::PT_CUBE );
		zAxisCylinderEntity->setMaterialName("BlueMaterial");
		zAxisCylinderNode->attachObject(zAxisCylinderEntity);
		zAxisCylinderNode->scale(vecToUnitCube);
		zAxisCylinderNode->scale(Ogre::Vector3(1.0,1.0,fDepth));
		zAxisCylinderNode->translate(Ogre::Vector3(0.0,0.0,fHalfDepth));

		//Create remainder of box		
		Ogre::ManualObject* remainingBox = m_pOgreSceneManager->createManualObject("Remaining Box");
		remainingBox->begin("BaseWhiteNoLighting",Ogre::RenderOperation::OT_LINE_LIST);
		remainingBox->position(0.0,		0.0,		0.0);		remainingBox->position(0.0,		0.0,		fDepth);
		remainingBox->position(0.0,		fHeight,	0.0);		remainingBox->position(0.0,		fHeight,	fDepth);
		remainingBox->position(fWidth,	0.0,		0.0);		remainingBox->position(fWidth,	0.0,		fDepth);
		remainingBox->position(fWidth,	fHeight,	0.0);		remainingBox->position(fWidth,	fHeight,	fDepth);

		remainingBox->position(0.0,		0.0,		0.0);		remainingBox->position(0.0,		fHeight,	0.0);
		remainingBox->position(0.0,		0.0,		fDepth);	remainingBox->position(0.0,		fHeight,	fDepth);
		remainingBox->position(fWidth,	0.0,		0.0);		remainingBox->position(fWidth,	fHeight,	0.0);
		remainingBox->position(fWidth,	0.0,		fDepth);	remainingBox->position(fWidth,	fHeight,	fDepth);

		remainingBox->position(0.0,		0.0,		0.0);		remainingBox->position(fWidth,	0.0,		0.0);
		remainingBox->position(0.0,		0.0,		fDepth);	remainingBox->position(fWidth,	0.0,		fDepth);
		remainingBox->position(0.0,		fHeight,	0.0);		remainingBox->position(fWidth,	fHeight,	0.0);
		remainingBox->position(0.0,		fHeight,	fDepth);	remainingBox->position(fWidth,	fHeight,	fDepth);
		remainingBox->end();
		Ogre::SceneNode *remainingBoxNode = m_axisNode->createChildSceneNode();
		remainingBoxNode->attachObject(remainingBox);
	}

	void ThermiteGameLogic::reloadShaders(void)
	{
		Ogre::ResourceManager::ResourceMapIterator I = Ogre::HighLevelGpuProgramManager::getSingleton().getResourceIterator();
		while (I.hasMoreElements()) {
			Ogre::ResourcePtr resource = I.getNext();
			resource->reload();
		}
	}

	void ThermiteGameLogic::initialisePhysics(void)
	{
#ifdef ENABLE_BULLET_PHYSICS
		//const Ogre::Vector3 gravityVector = Ogre::Vector3 (0,-98.1,0);
		//const Ogre::AxisAlignedBox bounds = Ogre::AxisAlignedBox (Ogre::Vector3 (-10000, -10000, -10000),Ogre::Vector3 (10000,  10000,  10000));
		//m_pOgreBulletWorld = new DynamicsWorld(mMap->m_pOgreSceneManager, bounds, gravityVector);
#endif //ENABLE_BULLET_PHYSICS
	}

	void ThermiteGameLogic::updatePolyVoxGeometry()
	{
		if(mMap == 0)
			return;

		if(!mMap->volumeResource.isNull())
		{		
			//Some values we'll need later.
			std::uint16_t regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();
			std::uint16_t halfRegionSideLength = regionSideLength / 2;
			std::uint16_t volumeWidthInRegions = mMap->volumeResource->getVolume()->getWidth() / regionSideLength;
			std::uint16_t volumeHeightInRegions = mMap->volumeResource->getVolume()->getHeight() / regionSideLength;
			std::uint16_t volumeDepthInRegions = mMap->volumeResource->getVolume()->getDepth() / regionSideLength;

			//Iterate over each region in the VolumeChangeTracker
			for(std::uint16_t regionZ = 0; regionZ < volumeDepthInRegions; ++regionZ)
			{		
				for(std::uint16_t regionY = 0; regionY < volumeHeightInRegions; ++regionY)
				{
					for(std::uint16_t regionX = 0; regionX < volumeWidthInRegions; ++regionX)
					{
						//Compute the extents of the current region
						const std::uint16_t firstX = regionX * regionSideLength;
						const std::uint16_t firstY = regionY * regionSideLength;
						const std::uint16_t firstZ = regionZ * regionSideLength;

						const std::uint16_t lastX = firstX + regionSideLength;
						const std::uint16_t lastY = firstY + regionSideLength;
						const std::uint16_t lastZ = firstZ + regionSideLength;	

						const float centreX = firstX + halfRegionSideLength;
						const float centreY = firstY + halfRegionSideLength;
						const float centreZ = firstZ + halfRegionSideLength;

						//The regions distance from the camera is used for prioritizing surface extraction
						Ogre::Vector3 cameraPos = mOgreCamera->getPosition();
						Ogre::Vector3 centre(centreX, centreY, centreZ);
						double distanceFromCameraSquared = (cameraPos - centre).squaredLength();

						//There's no guarentee that the MapRegion exists at this point...
						MapRegion* pMapRegion = mMap->m_volMapRegions->getVoxelAt(regionX, regionY, regionZ);

						//If the region has changed then we may need to add or remove MapRegion to/from the scene graph
						std::uint32_t uRegionTimeStamp = mMap->volumeChangeTracker->getLastModifiedTimeForRegion(regionX, regionY, regionZ);
						if(uRegionTimeStamp > mMap->m_volRegionTimeStamps->getVoxelAt(regionX,regionY,regionZ))
						{
							mMap->m_volRegionBeingProcessed->setVoxelAt(regionX,regionY,regionZ,true);

							//Convert to a real PolyVox::Region
							Vector3DInt16 v3dLowerCorner(firstX,firstY,firstZ);
							Vector3DInt16 v3dUpperCorner(lastX,lastY,lastZ);
							PolyVox::Region region(v3dLowerCorner, v3dUpperCorner);
							region.cropTo(mMap->volumeChangeTracker->getWrappedVolume()->getEnclosingRegion());

							//The prioirty ensures that the surfaces for regions close to the
							//camera get extracted before those which are distant from the camera.
							std::uint32_t uPriority = std::numeric_limits<std::uint32_t>::max() - static_cast<std::uint32_t>(distanceFromCameraSquared);

							//Extract the region
							SurfaceExtractorTaskData taskData(region, uRegionTimeStamp);
							SurfaceMeshExtractionTask* surfaceMeshExtractionTask = new SurfaceMeshExtractionTask(taskData, this);
							QObject::connect(surfaceMeshExtractionTask, SIGNAL(finished(SurfaceExtractorTaskData)), this, SLOT(uploadSurfaceExtractorResult(SurfaceExtractorTaskData)), Qt::QueuedConnection);
							QThreadPool::globalInstance()->start(surfaceMeshExtractionTask, uPriority);
							m_iNoSubmitted++;

							//Indicate that we've processed this region
							mMap->m_volRegionTimeStamps->setVoxelAt(regionX,regionY,regionZ,mMap->volumeChangeTracker->getLastModifiedTimeForRegion(regionX, regionY, regionZ));
						}
					}
				}
			}
		}
	}

	void ThermiteGameLogic::uploadSurfaceExtractorResult(SurfaceExtractorTaskData result)
	{
		std::uint16_t regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();

		//Determine where it came from
		uint16_t regionX = result.getRegion().getLowerCorner().getX() / regionSideLength;
		uint16_t regionY = result.getRegion().getLowerCorner().getY() / regionSideLength;
		uint16_t regionZ = result.getRegion().getLowerCorner().getZ() / regionSideLength;

		std::uint32_t uRegionTimeStamp = mMap->volumeChangeTracker->getLastModifiedTimeForRegion(regionX, regionY, regionZ);
		if(uRegionTimeStamp > result.m_uTimeStamp)
		{
			// The volume has changed since the command to generate this mesh was issued.
			// Just ignore it, and a correct version should be along soon...
			return;
		}

		uploadSurfaceMesh(result.getSurfaceMesh(), result.getRegion());


		SurfaceMeshDecimationTask* pOldSurfaceDecimator = mMap->m_volSurfaceDecimators->getVoxelAt(regionX, regionY, regionZ);

		m_backgroundThread->removeTask(pOldSurfaceDecimator);

		SurfaceMeshDecimationTask* surfaceMeshDecimationTask = new SurfaceMeshDecimationTask(result, this);
		QObject::connect(surfaceMeshDecimationTask, SIGNAL(finished(SurfaceExtractorTaskData)), this, SLOT(uploadSurfaceDecimatorResult(SurfaceExtractorTaskData)), Qt::QueuedConnection);

		mMap->m_volSurfaceDecimators->setVoxelAt(regionX, regionY, regionZ, surfaceMeshDecimationTask);

		m_backgroundThread->addTask(surfaceMeshDecimationTask);
	}

	void ThermiteGameLogic::uploadSurfaceDecimatorResult(SurfaceExtractorTaskData result)
	{
		std::uint16_t regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();

		//Determine where it came from
		uint16_t regionX = result.getRegion().getLowerCorner().getX() / regionSideLength;
		uint16_t regionY = result.getRegion().getLowerCorner().getY() / regionSideLength;
		uint16_t regionZ = result.getRegion().getLowerCorner().getZ() / regionSideLength;

		std::uint32_t uRegionTimeStamp = mMap->volumeChangeTracker->getLastModifiedTimeForRegion(regionX, regionY, regionZ);
		if(uRegionTimeStamp > result.m_uTimeStamp)
		{
			// The volume has changed since the command to generate this mesh was issued.
			// Just ignore it, and a correct version should be along soon...
			return;
		}

		uploadSurfaceMesh(result.getSurfaceMesh(), result.getRegion());
	}

	void ThermiteGameLogic::uploadSurfaceMesh(shared_ptr<SurfaceMesh> mesh, PolyVox::Region region)
	{
		bool bSimulatePhysics = qApp->settings()->value("Physics/SimulatePhysics", false).toBool();
		std::uint16_t regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();

		//Determine where it came from
		uint16_t regionX = region.getLowerCorner().getX() / regionSideLength;
		uint16_t regionY = region.getLowerCorner().getY() / regionSideLength;
		uint16_t regionZ = region.getLowerCorner().getZ() / regionSideLength;

		/*uint32_t uRegionTimeStamp = volumeChangeTracker->getLastModifiedTimeForRegion(regionX, regionY, regionZ);
		if(uRegionTimeStamp > result.m_uTimeStamp)
		{
			// The volume has changed since the command to generate this mesh was issued.
			// Just ignore it, and a correct version should be along soon...
			return;
		}*/

		//Create a MapRegion for that location if we don't have one already
		MapRegion* pMapRegion = mMap->m_volMapRegions->getVoxelAt(regionX, regionY, regionZ);
		if(pMapRegion == 0)
		{
			pMapRegion = new MapRegion(mMap, region.getLowerCorner());
			mMap->m_volMapRegions->setVoxelAt(regionX, regionY, regionZ, pMapRegion);
		}

		//Clear any previous geometry
		pMapRegion->removeAllSurfacePatchRenderables();

		//Get the SurfaceMesh and check it's valid
		shared_ptr<SurfaceMesh> meshWhole = mesh;
		if((meshWhole) && (meshWhole->isEmpty() == false))
		{			
			//The SurfaceMesh needs to be broken into pieces - one for each material. Iterate over the mateials...
			for(std::map< std::string, std::set<uint8_t> >::iterator iter = mMap->m_mapMaterialIds.begin(); iter != mMap->m_mapMaterialIds.end(); iter++)
			{
				//Get the properties
				std::string materialName = iter->first;
				std::set<std::uint8_t> voxelValues = iter->second;

				//Extract the part of the InexedSurfacePatch which corresponds to that material
				shared_ptr<SurfaceMesh> meshSubset = meshWhole->extractSubset(voxelValues);

				//And add it to the MapRegion
				pMapRegion->addSurfacePatchRenderable(materialName, *meshSubset);
			}

			//If we are simulating physics...
			if(bSimulatePhysics)
			{
				//Update the physics geometry
				pMapRegion->setPhysicsData(*(meshWhole.get()));
			}
		}

		//Update the progress bar
		m_iNoProcessed++;
		float fProgress = static_cast<float>(m_iNoProcessed) / static_cast<float>(m_iNoSubmitted);
		m_loadingProgress->setExtractingSurfacePercentageDone(fProgress*100);

		//If we've finished, the progress bar can be hidden.
		if((bLoadComplete == false) && (fProgress > 0.999))
		{
			m_loadingProgress->hide();
			bLoadComplete = true;
		}

		mMap->m_volRegionBeingProcessed->setVoxelAt(regionX,regionY,regionZ,false);
	}

	void ThermiteGameLogic::initScriptEngine(void)
	{
		scriptEngine = new QScriptEngine;

		QStringList extensions;
		extensions << "qt.core"
				   << "qt.gui"
				   << "qt.xml"
				   << "qt.svg"
				   << "qt.network"
				   << "qt.sql"
				   << "qt.opengl"
				   << "qt.webkit"
				   << "qt.xmlpatterns"
				   << "qt.uitools";
		QStringList failExtensions;
		foreach (const QString &ext, extensions)
		{
			QScriptValue ret = scriptEngine->importExtension(ext);
			if (ret.isError())
			{
				failExtensions.append(ext);
			}
		}
		if (!failExtensions.isEmpty())
		{
			if (failExtensions.size() == extensions.size())
			{
				qWarning("Failed to import Qt bindings!\n"
						 "Plugins directory searched: %s/script\n"
						 "Make sure that the bindings have been built, "
						 "and that this executable and the plugins are "
						 "using compatible Qt libraries.", qPrintable(qApp->libraryPaths().join(", ")));
			}
			else
			{
				qWarning("Failed to import some Qt bindings: %s\n"
						 "Plugins directory searched: %s/script\n"
						 "Make sure that the bindings have been built, "
						 "and that this executable and the plugins are "
						 "using compatible Qt libraries.",
						 qPrintable(failExtensions.join(", ")), qPrintable(qApp->libraryPaths().join(", ")));
			}
		}
		else
		{
			qDebug("All Qt bindings loaded successfully.");
		}
	}

	void ThermiteGameLogic::initScriptEnvironment(void)
	{
		mGlobals = new Globals(this);

		QScriptValue lightClass = scriptEngine->scriptValueFromQMetaObject<Light>();
		scriptEngine->globalObject().setProperty("Light", lightClass);

		QScriptValue entityClass = scriptEngine->scriptValueFromQMetaObject<Entity>();
		scriptEngine->globalObject().setProperty("Entity", entityClass);

		QScriptValue mapClass = scriptEngine->scriptValueFromQMetaObject<Map>();
		scriptEngine->globalObject().setProperty("Map", mapClass);

		QScriptValue globalsScriptValue = scriptEngine->newQObject(mGlobals);
		scriptEngine->globalObject().setProperty("globals", globalsScriptValue);

		QScriptValue keyboardScriptValue = scriptEngine->newQObject(&keyboard);
		scriptEngine->globalObject().setProperty("keyboard", keyboardScriptValue);

		QScriptValue mouseScriptValue = scriptEngine->newQObject(mouse);
		scriptEngine->globalObject().setProperty("mouse", mouseScriptValue);

		QScriptValue cameraScriptValue = scriptEngine->newQObject(camera);
		scriptEngine->globalObject().setProperty("camera", cameraScriptValue);

		QScriptValue Qt = scriptEngine->newQMetaObject(&staticQtMetaObject);
		Qt.setProperty("App", scriptEngine->newQObject(qApp));
		scriptEngine->globalObject().setProperty("Qt", Qt);

		QScriptValue objectStoreScriptValue = scriptEngine->newQObject(&mObjectStore);
		scriptEngine->globalObject().setProperty("objectStore", objectStoreScriptValue);
	}

	void ThermiteGameLogic::startScriptingEngine(void)
	{
		m_bRunScript = true;
	}

	void ThermiteGameLogic::stopScriptingEngine(void)
	{
		m_bRunScript = false;
	}
}