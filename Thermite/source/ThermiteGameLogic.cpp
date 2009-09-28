#include "ThermiteGameLogic.h"

#include "MainMenu.h"
#include "Shell.h"
#include "LoadMapWidget.h"

#include "LogManager.h"
#include "OgreWidget.h"
#include "MultiThreadedSurfaceExtractor.h"
#include "SurfacePatchRenderable.h"
#include "MapManager.h"
#include "VolumeManager.h"

#include <OgreEntity.h>
#include <OgreRenderWindow.h>
#include <OgreResourceGroupManager.h>
#include <OgreRoot.h>

#include <QDirIterator>
#include <QKeyEvent>
#include <QMovie>
#include <QSettings>

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

using PolyVox::uint32_t;
using PolyVox::uint16_t;
using PolyVox::uint8_t;

namespace Thermite
{
	ThermiteGameLogic::ThermiteGameLogic(void)
		:GameLogic()
		,mCurrentFrameNumber(0)
		,mMap(0)
	{
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
		mSceneManager = new Ogre::DefaultSceneManager("EngineSceneManager");
		Ogre::Camera* dummyCamera = mSceneManager->createCamera("DummyCamera");
		mSceneManager->getRootSceneNode()->attachObject(dummyCamera);
		mApplication->ogreRenderWindow()->addViewport(dummyCamera)->setBackgroundColour(Ogre::ColourValue::Black);

		//Set up and start the thermite logo animation. This plays while we initialise.
		m_pThermiteLogoMovie = new QMovie(QString::fromUtf8(":/animations/thermite_logo.mng"));
		m_pThermiteLogoLabel = new QLabel(qApp->mainWidget(), Qt::FramelessWindowHint | Qt::Tool);
		m_pThermiteLogoLabel->setMovie(m_pThermiteLogoMovie);
		m_pThermiteLogoMovie->jumpToFrame(0);
		m_pThermiteLogoLabel->resize(m_pThermiteLogoMovie->currentImage().size());
		m_pThermiteLogoLabel->show();
		m_pThermiteLogoMovie->start();

		//Load the Cg plugin
		#if defined(_DEBUG)
			Ogre::Root::getSingletonPtr()->loadPlugin("Plugin_CgProgramManager_d");
		#else
			Ogre::Root::getSingletonPtr()->loadPlugin("Plugin_CgProgramManager");
		#endif

		//Initialise all resources
		addResourceDirectory("./resources/");
		char* strAppName = m_commandLineArgs.getValue("appname");
		if(strAppName != 0)
		{
			addResourceDirectory(QString("../share/thermite/apps/") + QString::fromAscii(strAppName));
		}
		Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

		//Set up various GUI components...
		//The load map widget
		LoadMapWidget* wgtLoadMap = new LoadMapWidget(this, qApp->mainWidget(), Qt::Tool);
		Application::centerWidget(wgtLoadMap, qApp->mainWidget());

		//Used to display loading/extraction progress
		m_loadingProgress = new LoadingProgress(qApp->mainWidget(), Qt::Tool);
		Application::centerWidget(m_loadingProgress, qApp->mainWidget());

		//The main menu
		mMainMenu = new MainMenu(qApp->mainWidget(), Qt::Tool);
		Application::centerWidget(mMainMenu, qApp->mainWidget());
		QObject::connect(mMainMenu, SIGNAL(resumeClicked(void)), mMainMenu, SLOT(hide(void)));
		QObject::connect(mMainMenu, SIGNAL(quitClicked(void)), qApp->mainWidget(), SLOT(close(void)));
		QObject::connect(mMainMenu, SIGNAL(settingsClicked(void)), mApplication, SLOT(showSettingsDialog(void)));
		QObject::connect(mMainMenu, SIGNAL(viewLogsClicked(void)), mApplication, SLOT(showLogManager(void)));
		QObject::connect(mMainMenu, SIGNAL(loadClicked(void)), wgtLoadMap, SLOT(show(void)));

		//Show the main menu after the animation has finished
		QTimer::singleShot(2000, mMainMenu, SLOT(show()));

		//Set the frame sate to be as high as possible
		mApplication->setAutoUpdateInterval(0);

		//Some Ogre related stuff we need to set up
		Ogre::Root::getSingletonPtr()->addMovableObjectFactory(new SurfacePatchRenderableFactory);
		VolumeManager* vm = new VolumeManager;
		vm->m_pProgressListener = new VolumeSerializationProgressListenerImpl(this);

		MapManager* mm = new MapManager;
		mm->m_pThermiteGameLogic = this;
		mm->m_pOgreSceneManager = mSceneManager;

		//From here on, I'm not sure it belongs in this initialise function... maybe in Map?		

		if(qApp->settings()->value("Shadows/EnableShadows", false).toBool())
		{
			mSceneManager->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED);
			//m_pOgreSceneManager->setShadowFarDistance(1000.0f);
			mSceneManager->setShadowTextureSelfShadow(true);
			mSceneManager->setShadowTextureCasterMaterial("ShadowMapCasterMaterial");
			mSceneManager->setShadowTexturePixelFormat(Ogre::PF_FLOAT32_R);
			mSceneManager->setShadowCasterRenderBackFaces(true);
			mSceneManager->setShadowTextureSize(qApp->settings()->value("Shadows/ShadowMapSize", 1024).toInt());
		}	

		mTime = new QTime;
		mTime->start();

		mCurrentFrameNumber = 0;

		mCameraSpeed = 50.0;
		mCameraRotationalSpeed = 0.01;		
	}

	void ThermiteGameLogic::update(void)
	{
		if(mMap == 0)
			return;

		mLastFrameTime = mCurrentTime;
		mCurrentTime = mTime->elapsed();

		float timeElapsedInSeconds = (mCurrentTime - mLastFrameTime) / 1000.0f;

		float distance = mCameraSpeed * timeElapsedInSeconds;

		if(mKeyStates[Qt::Key_W] == KS_PRESSED)
		{
			mCamera->setPosition(mCamera->getPosition() + mCamera->getDirection() * distance);
		}
		if(mKeyStates[Qt::Key_S] == KS_PRESSED)
		{
			mCamera->setPosition(mCamera->getPosition() - mCamera->getDirection() * distance);
		}
		if(mKeyStates[Qt::Key_A] == KS_PRESSED)
		{
			mCamera->setPosition(mCamera->getPosition() - mCamera->getRight() * distance);
		}
		if(mKeyStates[Qt::Key_D] == KS_PRESSED)
		{
			mCamera->setPosition(mCamera->getPosition() + mCamera->getRight() * distance);
		}

		if(mCurrentFrameNumber != 0)
		{
			QPoint mouseDelta = mCurrentMousePos - mLastFrameMousePos;
			mCamera->yaw(Ogre::Radian(-mouseDelta.x() * mCameraRotationalSpeed));
			mCamera->pitch(Ogre::Radian(-mouseDelta.y() * mCameraRotationalSpeed));

			int wheelDelta = mCurrentWheelPos - mLastFrameWheelPos;
			Ogre::Radian fov = mCamera->getFOVy();
			fov += Ogre::Radian(-wheelDelta * 0.001);
			fov = (std::min)(fov, Ogre::Radian(2.0f));
			fov = (std::max)(fov, Ogre::Radian(0.5f));
			mCamera->setFOVy(fov);
		}
		mLastFrameMousePos = mCurrentMousePos;
		mLastFrameWheelPos = mCurrentWheelPos;

		//Update the cannon
		if(mTurretNode && mGunNode)
		{
			float directionInDegrees = mCannonController->direction();
			float elevationInDegrees = mCannonController->elevation();
			mTurretNode->setOrientation(mTurretOriginalOrientation);
			mGunNode->setOrientation(mGunOriginalOrientation);

			mTurretNode->rotate(Ogre::Vector3(0.0,1.0,0.0), Ogre::Radian(directionInDegrees / 57.0));
			mGunNode->rotate(Ogre::Vector3(0.0,0.0,1.0), Ogre::Radian(elevationInDegrees / 57.0)); //Elevation
		}

		//The fun stuff!
		updatePolyVoxGeometry();
		
#ifdef ENABLE_BULLET_PHYSICS
		if((qApp->settings()->value("Physics/SimulatePhysics", false).toBool()) && (bLoadComplete))
		{
			m_pOgreBulletWorld->stepSimulation(timeElapsedInSeconds, 10);
		}
#endif //ENABLE_BULLET_PHYSICS

		list<Shell*> shellsToDelete;

		for(list<Shell*>::iterator iter = m_listShells.begin(); iter != m_listShells.end(); iter++)
		{
			(*iter)->update(timeElapsedInSeconds);
			Ogre::Vector3 shellPos = (*iter)->m_pSceneNode->getPosition();

			if(mMap->volumeResource->getVolume()->getEnclosingRegion().containsPoint(PolyVox::Vector3DFloat(shellPos.x, shellPos.y, shellPos.z), 1.0))
			{
				if(mMap->volumeResource->getVolume()->getVoxelAt(shellPos.x, shellPos.y, shellPos.z) != 0)
				{
					createSphereAt(PolyVox::Vector3DFloat(shellPos.x, shellPos.y, shellPos.z), 50, 0);
					shellsToDelete.push_back(*iter);
				}
			}
		}

		for(list<Shell*>::iterator iter = shellsToDelete.begin(); iter != shellsToDelete.end(); iter++)
		{
			m_listShells.remove(*iter);
			delete (*iter);
		}

		++mCurrentFrameNumber;
	}

	void ThermiteGameLogic::createSphereAt(PolyVox::Vector3DFloat centre, float radius, PolyVox::uint8_t value)
{
	int firstX = static_cast<int>(std::floor(centre.getX() - radius));
	int firstY = static_cast<int>(std::floor(centre.getY() - radius));
	int firstZ = static_cast<int>(std::floor(centre.getZ() - radius));

	int lastX = static_cast<int>(std::ceil(centre.getX() + radius));
	int lastY = static_cast<int>(std::ceil(centre.getY() + radius));
	int lastZ = static_cast<int>(std::ceil(centre.getZ() + radius));

	float radiusSquared = radius * radius;

	//Check bounds
	firstX = std::max(firstX,0);
	firstY = std::max(firstY,0);
	firstZ = std::max(firstZ,0);

	lastX = std::min(lastX,int(volumeChangeTracker->getWrappedVolume()->getWidth()-1));
	lastY = std::min(lastY,int(volumeChangeTracker->getWrappedVolume()->getHeight()-1));
	lastZ = std::min(lastZ,int(volumeChangeTracker->getWrappedVolume()->getDepth()-1));

	volumeChangeTracker->lockRegion(PolyVox::Region(PolyVox::Vector3DInt16(firstX, firstY, firstZ), PolyVox::Vector3DInt16(lastX, lastY, lastZ)));
	for(int z = firstZ; z <= lastZ; ++z)
	{
		for(int y = firstY; y <= lastY; ++y)
		{
			for(int x = firstX; x <= lastX; ++x)
			{
				if((centre - PolyVox::Vector3DFloat(x,y,z)).lengthSquared() <= radiusSquared)
				{
					volumeChangeTracker->setLockedVoxelAt(x,y,z,value);
				}
			}
		}
	}
	volumeChangeTracker->unlockRegion();
}

	void ThermiteGameLogic::shutdown(void)
	{
		Ogre::Root::getSingleton().destroySceneManager(mSceneManager);
	}

	void ThermiteGameLogic::onKeyPress(QKeyEvent* event)
	{
		mKeyStates[event->key()] = KS_PRESSED;

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
		mKeyStates[event->key()] = KS_RELEASED;
	}

	void ThermiteGameLogic::onMousePress(QMouseEvent* event)
	{
		mCurrentMousePos = event->pos();
		mLastFrameMousePos = mCurrentMousePos;
	}

	void ThermiteGameLogic::onMouseMove(QMouseEvent* event)
	{
		mCurrentMousePos = event->pos();
	}

	void ThermiteGameLogic::onWheel(QWheelEvent* event)
	{
		mCurrentWheelPos += event->delta();
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

	void ThermiteGameLogic::fireCannon(void)
	{
		Shell* shell = new Shell(mMap, mGunNode->_getDerivedPosition(), Ogre::Vector3(0.0f, 1.0f, 0.0f));

		//Update the shell
		float directionInDegrees = mCannonController->direction();
		float elevationInDegrees = mCannonController->elevation();
		shell->m_pSceneNode->setOrientation(mGunOriginalOrientation);
		shell->m_pSceneNode->rotate(Ogre::Vector3(0.0,1.0,0.0),Ogre::Radian(1.57));
		shell->m_pSceneNode->rotate(Ogre::Vector3(0.0,-1.0,0.0), Ogre::Radian(directionInDegrees / 57.0));
		shell->m_pSceneNode->rotate(Ogre::Vector3(1.0,0.0,0.0), Ogre::Radian(elevationInDegrees / 57.0)); //Elevation

		shell->m_vecVelocity = shell->m_pSceneNode->getLocalAxes().GetColumn(2);
		shell->m_vecVelocity.normalise();
		shell->m_vecVelocity *= 100.0f;

		m_listShells.push_back(shell);
	}

	void ThermiteGameLogic::loadMap(QString strMapName)
	{
		bLoadComplete = false;
		m_pThermiteLogoLabel->hide();
		mMainMenu->hide();

		//Temporary hack until loading new map is fixed...
		mMainMenu->disableLoadButton();

		//The QtOgre DotScene loading code will clear the existing scene except for cameras, as these
		//could be used by existing viewports. Therefore we clear and viewports and cameras before
		//calling the loading code.
		mApplication->ogreRenderWindow()->removeAllViewports();
		mSceneManager->destroyAllCameras();

		m_loadingProgress->show();

		/*mMap = new Map(Ogre::Vector3 (0,0,-98.1),Ogre::AxisAlignedBox (Ogre::Vector3 (-10000, -10000, -10000),Ogre::Vector3 (10000,  10000,  10000)), 0.1f, mSceneManager);
		mMap->m_pThermiteGameLogic = this;
		mMap->loadScene(strMapName.toStdString());*/

		m_iNoProcessed = 0;
		m_iNoSubmitted = 0;

		MapResourcePtr mapResource = MapManager::getSingletonPtr()->load(strMapName.toStdString(), "General");
		if(mapResource.isNull())
		{
			Ogre::LogManager::getSingleton().logMessage("Failed to load map");
		}

		int regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();
		int volumeWidthInRegions = volumeChangeTracker->getWrappedVolume()->getWidth() / regionSideLength;
		int volumeHeightInRegions = volumeChangeTracker->getWrappedVolume()->getHeight() / regionSideLength;
		int volumeDepthInRegions = volumeChangeTracker->getWrappedVolume()->getDepth() / regionSideLength;

		m_volRegionTimeStamps = new Volume<uint32_t>(volumeWidthInRegions, volumeHeightInRegions, volumeDepthInRegions, 0);
		m_volMapRegions = new Volume<MapRegion*>(volumeWidthInRegions, volumeHeightInRegions, volumeDepthInRegions, 0);

		volumeChangeTracker = new VolumeChangeTracker(mMap->volumeResource->getVolume(), regionSideLength);
		volumeChangeTracker->setAllRegionsModified();

		m_pMTSE = new MultiThreadedSurfaceExtractor(volumeChangeTracker->getWrappedVolume(), qApp->settings()->value("Engine/NoOfSurfaceExtractionThreads", 2).toInt());
		m_pMTSE->start();

		mMap = mapResource->m_pMap;

		initialisePhysics();

		if(qApp->settings()->value("Debug/ShowVolumeAxes", false).toBool())
		{
			createAxis(mMap->volumeResource->getVolume()->getWidth(), mMap->volumeResource->getVolume()->getHeight(), mMap->volumeResource->getVolume()->getDepth());
		}

		//This gets the first camera which was found in the scene.
		Ogre::SceneManager::CameraIterator camIter = mSceneManager->getCameraIterator();
		mCamera = camIter.peekNextValue();

		mApplication->ogreRenderWindow()->addViewport(mCamera)->setBackgroundColour(Ogre::ColourValue::Black);

		try
		{
			mTurretNode = dynamic_cast<Ogre::SceneNode*>(mSceneManager->getRootSceneNode()->getChild("chassis")->getChild("turret_main"));
			mGunNode = dynamic_cast<Ogre::SceneNode*>(mSceneManager->getRootSceneNode()->getChild("chassis")->getChild("turret_main")->getChild("gun_main"));

			mTurretOriginalOrientation = mTurretNode->getOrientation();
			mGunOriginalOrientation = mGunNode->getOrientation();

			mCannonController = new CannonController(this, qApp->mainWidget(), Qt::Tool);
			mCannonController->move(qApp->mainWidget()->geometry().left() + qApp->mainWidget()->geometry().width() - mCannonController->frameGeometry().width() - 10, qApp->mainWidget()->geometry().top() + 10);
			mCannonController->show();
		}
		catch(Ogre::ItemIdentityException&) //Thrown if the tank is not found
		{
			mTurretNode = 0;
			mGunNode = 0;
		}

		//m_loadingProgress->hide();

		mApplication->showFPSCounter();
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
		m_axisNode = mSceneManager->getRootSceneNode()->createChildSceneNode();

		//Create sphere representing origin
		Ogre::SceneNode* originNode = m_axisNode->createChildSceneNode();
		Ogre::Entity *originSphereEntity = mSceneManager->createEntity( "Origin Sphere", Ogre::SceneManager::PT_CUBE );
		originSphereEntity->setMaterialName("OriginMaterial");
		originNode->attachObject(originSphereEntity);
		originNode->scale(vecToUnitCube);
		originNode->scale(fOriginSize,fOriginSize,fOriginSize);

		//Create x-axis
		Ogre::SceneNode *xAxisCylinderNode = m_axisNode->createChildSceneNode();
		Ogre::Entity *xAxisCylinderEntity = mSceneManager->createEntity( "X Axis", Ogre::SceneManager::PT_CUBE );
		xAxisCylinderEntity->setMaterialName("XAxisMaterial");
		xAxisCylinderNode->attachObject(xAxisCylinderEntity);	
		xAxisCylinderNode->scale(vecToUnitCube);
		xAxisCylinderNode->scale(Ogre::Vector3(fWidth,1.0,1.0));
		xAxisCylinderNode->translate(Ogre::Vector3(fHalfWidth,0.0,0.0));

		//Create y-axis
		Ogre::SceneNode *yAxisCylinderNode = m_axisNode->createChildSceneNode();
		Ogre::Entity *yAxisCylinderEntity = mSceneManager->createEntity( "Y Axis", Ogre::SceneManager::PT_CUBE );
		yAxisCylinderEntity->setMaterialName("YAxisMaterial");
		yAxisCylinderNode->attachObject(yAxisCylinderEntity);		
		yAxisCylinderNode->scale(vecToUnitCube);
		yAxisCylinderNode->scale(Ogre::Vector3(1.0,fHeight,1.0));
		yAxisCylinderNode->translate(Ogre::Vector3(0.0,fHalfHeight,0.0));

		//Create z-axis
		Ogre::SceneNode *zAxisCylinderNode = m_axisNode->createChildSceneNode();
		Ogre::Entity *zAxisCylinderEntity = mSceneManager->createEntity( "Z Axis", Ogre::SceneManager::PT_CUBE );
		zAxisCylinderEntity->setMaterialName("ZAxisMaterial");
		zAxisCylinderNode->attachObject(zAxisCylinderEntity);
		zAxisCylinderNode->scale(vecToUnitCube);
		zAxisCylinderNode->scale(Ogre::Vector3(1.0,1.0,fDepth));
		zAxisCylinderNode->translate(Ogre::Vector3(0.0,0.0,fHalfDepth));

		//Create remainder of box		
		Ogre::ManualObject* remainingBox = mSceneManager->createManualObject("Remaining Box");
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
		const Ogre::Vector3 gravityVector = Ogre::Vector3 (0,-98.1,0);
		const Ogre::AxisAlignedBox bounds = Ogre::AxisAlignedBox (Ogre::Vector3 (-10000, -10000, -10000),Ogre::Vector3 (10000,  10000,  10000));
		m_pOgreBulletWorld = new DynamicsWorld(mMap->m_pOgreSceneManager, bounds, gravityVector);
#endif //ENABLE_BULLET_PHYSICS
	}

	void ThermiteGameLogic::updatePolyVoxGeometry()
	{
		if(!mMap->volumeResource.isNull())
		{		
			//Some values we'll need later.
			uint16_t regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();
			uint16_t halfRegionSideLength = regionSideLength / 2;
			uint16_t volumeWidthInRegions = mMap->volumeResource->getVolume()->getWidth() / regionSideLength;
			uint16_t volumeHeightInRegions = mMap->volumeResource->getVolume()->getHeight() / regionSideLength;
			uint16_t volumeDepthInRegions = mMap->volumeResource->getVolume()->getDepth() / regionSideLength;

			double fLod0ToLod1Boundary = qApp->settings()->value("Engine/Lod0ToLod1Boundary", 256.0f).toDouble();
			double fLod0ToLod1BoundarySquared = fLod0ToLod1Boundary * fLod0ToLod1Boundary;
			double fLod1ToLod2Boundary = qApp->settings()->value("Engine/Lod1ToLod2Boundary", 512.0f).toDouble();
			double fLod1ToLod2BoundarySquared = fLod1ToLod2Boundary * fLod1ToLod2Boundary;

			//Iterate over each region in the VolumeChangeTracker
			for(uint16_t regionZ = 0; regionZ < volumeDepthInRegions; ++regionZ)
			{		
				for(uint16_t regionY = 0; regionY < volumeHeightInRegions; ++regionY)
				{
					for(uint16_t regionX = 0; regionX < volumeWidthInRegions; ++regionX)
					{
						//Compute the extents of the current region
						const uint16_t firstX = regionX * regionSideLength;
						const uint16_t firstY = regionY * regionSideLength;
						const uint16_t firstZ = regionZ * regionSideLength;

						const uint16_t lastX = firstX + regionSideLength;
						const uint16_t lastY = firstY + regionSideLength;
						const uint16_t lastZ = firstZ + regionSideLength;	

						const float centreX = firstX + halfRegionSideLength;
						const float centreY = firstY + halfRegionSideLength;
						const float centreZ = firstZ + halfRegionSideLength;

						//The regions distance from the camera is used for
						//LOD selection and prioritizing surface extraction
						Ogre::Vector3 cameraPos = mCamera->getPosition();
						Ogre::Vector3 centre(centreX, centreY, centreZ);
						double distanceFromCameraSquared = (cameraPos - centre).squaredLength();

						//There's no guarentee that the MapRegion exists at this point...
						MapRegion* pMapRegion = m_volMapRegions->getVoxelAt(regionX, regionY, regionZ);
						if(pMapRegion)
						{							
							//But if it does, we set the appropriate LOD level based on distance from the camera.
							if((distanceFromCameraSquared > fLod1ToLod2BoundarySquared) && (fLod1ToLod2Boundary > 0.0f))
							{
								pMapRegion->setLodLevelToUse(2);
							}
							else if((distanceFromCameraSquared > fLod0ToLod1BoundarySquared) && (fLod0ToLod1Boundary > 0.0f))
							{
								pMapRegion->setLodLevelToUse(1);
							}
							else
							{
								pMapRegion->setLodLevelToUse(0);
							}
						}

						//If the region has changed then we may need to add or remove MapRegion to/from the scene graph
						if(volumeChangeTracker->getLastModifiedTimeForRegion(regionX, regionY, regionZ) > m_volRegionTimeStamps->getVoxelAt(regionX,regionY,regionZ))
						{
							//Convert to a real PolyVox::Region
							Vector3DInt16 v3dLowerCorner(firstX,firstY,firstZ);
							Vector3DInt16 v3dUpperCorner(lastX,lastY,lastZ);
							PolyVox::Region region(v3dLowerCorner, v3dUpperCorner);
							region.cropTo(volumeChangeTracker->getWrappedVolume()->getEnclosingRegion());

							//The prioirty ensures that the surfaces for regions close to the
							//camera getextracted before those which are distant from the camera.
							uint32_t uPriority = std::numeric_limits<uint32_t>::max() - static_cast<uint32_t>(distanceFromCameraSquared);

							//Extract the region with a LOD level of 0
							m_pMTSE->pushTask(SurfaceExtractorTaskData(region, 0, uPriority));
							m_iNoSubmitted++;

							if(fLod0ToLod1Boundary > 0) //If the first LOD level is enabled
							{
								//Extract the region with a LOD level of 1
								m_pMTSE->pushTask(SurfaceExtractorTaskData(region, 1, uPriority));
								m_iNoSubmitted++;
							}

							if(fLod1ToLod2Boundary > 0) //If the second LOD level is enabled
							{
								//Extract the region with a LOD level of 2
								m_pMTSE->pushTask(SurfaceExtractorTaskData(region, 2, uPriority));
								m_iNoSubmitted++;
							}

							//Indicate that we've processed this region
							m_volRegionTimeStamps->setVoxelAt(regionX,regionY,regionZ,volumeChangeTracker->getLastModifiedTimeForRegion(regionX, regionY, regionZ));
						}
					}
				}
			}

			bool bSimulatePhysics = qApp->settings()->value("Physics/SimulatePhysics", false).toBool();
			int iPhysicsLOD = qApp->settings()->value("Physics/PhysicsLOD", 0).toInt();

			//Process any results which have been returned by the surface extractor threads.
			while(m_pMTSE->noOfResultsAvailable() > 0)
			{
				//Get the next available result
				SurfaceExtractorTaskData result;
				result = m_pMTSE->popResult();

				//Determine where it came from
				PolyVox::uint16_t regionX = result.getRegion().getLowerCorner().getX() / regionSideLength;
				PolyVox::uint16_t regionY = result.getRegion().getLowerCorner().getY() / regionSideLength;
				PolyVox::uint16_t regionZ = result.getRegion().getLowerCorner().getZ() / regionSideLength;

				//Create a MapRegion for that location if we don't have one already
				MapRegion* pMapRegion = m_volMapRegions->getVoxelAt(regionX, regionY, regionZ);
				if(pMapRegion == 0)
				{
					pMapRegion = new MapRegion(mMap, result.getRegion().getLowerCorner());
					m_volMapRegions->setVoxelAt(regionX, regionY, regionZ, pMapRegion);
				}

				//Get the IndexedSurfacePatch and check it's valid
				POLYVOX_SHARED_PTR<IndexedSurfacePatch> ispWhole = result.getIndexedSurfacePatch();
				if((ispWhole) && (ispWhole->isEmpty() == false))
				{
					//Clear any previous geometry
					pMapRegion->removeAllSurfacePatchRenderablesForLod(result.getLodLevel());

					//The IndexedSurfacePatch needs to be broken into pieces - one for each material. Iterate over the mateials...
					for(std::map< std::string, std::set<PolyVox::uint8_t> >::iterator iter = mMap->m_mapMaterialIds.begin(); iter != mMap->m_mapMaterialIds.end(); iter++)
					{
						//Get the properties
						std::string materialName = iter->first;
						std::set<uint8_t> voxelValues = iter->second;

						//Extract the part of the InexedSurfacePatch which corresponds to that material
						POLYVOX_SHARED_PTR<IndexedSurfacePatch> ispSubset = ispWhole->extractSubset(voxelValues);

						//And add it to the MapRegion
						pMapRegion->addSurfacePatchRenderable(materialName, *ispSubset, result.getLodLevel());
					}

					//If we are simulating physics and the current LOD matches...
					if((bSimulatePhysics) && (result.getLodLevel() == iPhysicsLOD))
					{
						//Update the physics geometry
						pMapRegion->setPhysicsData(*(ispWhole.get()));
					}
				}

				//Update the progress bar
				m_iNoProcessed++;
				float fProgress = static_cast<float>(m_iNoProcessed) / static_cast<float>(m_iNoSubmitted);
				m_loadingProgress->setExtractingSurfacePercentageDone(fProgress*100);

				//If we've finished, the progress bar can be hidden.
				if(fProgress > 0.999)
				{
					m_loadingProgress->hide();
					bLoadComplete = true;
				}
			}
		}		
	}
}