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
#include "VolumeManager.h"

#include "Application.h"
#include "LogManager.h"

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

using PolyVox::uint32_t;
using PolyVox::uint16_t;
using PolyVox::uint8_t;

namespace Thermite
{
	ThermiteGameLogic::ThermiteGameLogic(void)
		:GameLogic()
		,mCurrentFrameNumber(0)
		,mMap(0)
		,m_pActiveOgreSceneManager(0)
		,m_pDummyOgreSceneManager(0)
	{
		qRegisterMetaType<SurfaceExtractorTaskData>("SurfaceExtractorTaskData");
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
		m_pDummyOgreSceneManager = new Ogre::DefaultSceneManager("DummySceneManager");
		Ogre::Camera* dummyCamera = m_pDummyOgreSceneManager->createCamera("DummyCamera");
		m_pDummyOgreSceneManager->getRootSceneNode()->attachObject(dummyCamera);
		mMainViewport = mApplication->ogreRenderWindow()->addViewport(dummyCamera);
		mMainViewport->setBackgroundColour(Ogre::ColourValue::Black);

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
	}

	void ThermiteGameLogic::update(void)
	{
		if(mMap == 0)
			return;

		mLastFrameTime = mCurrentTime;
		mCurrentTime = mTime->elapsed();

		mTimeElapsedInSeconds = (mCurrentTime - mLastFrameTime) / 1000.0f;

		/*float distance = mCameraSpeed * timeElapsedInSeconds;

		if(mKeyStates[Qt::Key_W] == KS_PRESSED)
		{
			mActiveCamera->setPosition(mActiveCamera->getPosition() + mActiveCamera->getDirection() * distance);
		}
		if(mKeyStates[Qt::Key_S] == KS_PRESSED)
		{
			mActiveCamera->setPosition(mActiveCamera->getPosition() - mActiveCamera->getDirection() * distance);
		}
		if(mKeyStates[Qt::Key_A] == KS_PRESSED)
		{
			mActiveCamera->setPosition(mActiveCamera->getPosition() - mActiveCamera->getRight() * distance);
		}
		if(mKeyStates[Qt::Key_D] == KS_PRESSED)
		{
			mActiveCamera->setPosition(mActiveCamera->getPosition() + mActiveCamera->getRight() * distance);
		}

		if(mCurrentFrameNumber != 0)
		{
			QPoint mouseDelta = mCurrentMousePos - mLastFrameMousePos;
			mActiveCamera->yaw(Ogre::Radian(-mouseDelta.x() * mCameraRotationalSpeed));
			mActiveCamera->pitch(Ogre::Radian(-mouseDelta.y() * mCameraRotationalSpeed));

			int wheelDelta = mCurrentWheelPos - mLastFrameWheelPos;
			Ogre::Radian fov = mActiveCamera->getFOVy();
			fov += Ogre::Radian(-wheelDelta * 0.001);
			fov = (std::min)(fov, Ogre::Radian(2.0f));
			fov = (std::max)(fov, Ogre::Radian(0.5f));
			mActiveCamera->setFOVy(fov);
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
		}*/

		//The fun stuff!
		updatePolyVoxGeometry();
		
#ifdef ENABLE_BULLET_PHYSICS
		if((qApp->settings()->value("Physics/SimulatePhysics", false).toBool()) && (bLoadComplete))
		{
			m_pOgreBulletWorld->stepSimulation(mTimeElapsedInSeconds, 10);
		}
#endif //ENABLE_BULLET_PHYSICS

		/*list<Shell*> shellsToDelete;

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
		}*/

		++mCurrentFrameNumber;
	}

	void ThermiteGameLogic::shutdown(void)
	{
		Ogre::Root::getSingleton().destroySceneManager(m_pDummyOgreSceneManager);
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

		MapResourcePtr mapResource = MapManager::getSingletonPtr()->load(strMapName.toStdString(), "General");
		if(mapResource.isNull())
		{
			Ogre::LogManager::getSingleton().logMessage("Failed to load map");
		}

		mMap = mapResource->m_pMap;
		m_pActiveOgreSceneManager = mMap->m_pOgreSceneManager;
		m_pOgreBulletWorld = mMap->m_pOgreBulletWorld;

		int regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();

		volumeChangeTracker = new VolumeChangeTracker(mMap->volumeResource->getVolume(), regionSideLength);
		volumeChangeTracker->setAllRegionsModified();

		int volumeWidthInRegions = volumeChangeTracker->getWrappedVolume()->getWidth() / regionSideLength;
		int volumeHeightInRegions = volumeChangeTracker->getWrappedVolume()->getHeight() / regionSideLength;
		int volumeDepthInRegions = volumeChangeTracker->getWrappedVolume()->getDepth() / regionSideLength;

		m_volRegionTimeStamps = new Volume<uint32_t>(volumeWidthInRegions, volumeHeightInRegions, volumeDepthInRegions, 0);
		m_volMapRegions = new Volume<MapRegion*>(volumeWidthInRegions, volumeHeightInRegions, volumeDepthInRegions, 0);
		m_volRegionBeingProcessed = new Volume<bool>(volumeWidthInRegions, volumeHeightInRegions, volumeDepthInRegions, 0);
		m_volSurfaceDecimators = new Volume<SurfaceMeshDecimationTask*>(volumeWidthInRegions, volumeHeightInRegions, volumeDepthInRegions, 0);

		mMap = mapResource->m_pMap;

		initialisePhysics();

		if(qApp->settings()->value("Debug/ShowVolumeAxes", false).toBool())
		{
			createAxis(mMap->volumeResource->getVolume()->getWidth(), mMap->volumeResource->getVolume()->getHeight(), mMap->volumeResource->getVolume()->getDepth());
		}

		if(qApp->settings()->value("Shadows/EnableShadows", false).toBool())
		{
			m_pActiveOgreSceneManager->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED);
			//m_pActiveOgreSceneManager->setShadowFarDistance(1000.0f);
			m_pActiveOgreSceneManager->setShadowTextureSelfShadow(true);
			m_pActiveOgreSceneManager->setShadowTextureCasterMaterial("ShadowMapCasterMaterial");
			m_pActiveOgreSceneManager->setShadowTexturePixelFormat(Ogre::PF_FLOAT32_R);
			m_pActiveOgreSceneManager->setShadowCasterRenderBackFaces(true);
			m_pActiveOgreSceneManager->setShadowTextureSize(qApp->settings()->value("Shadows/ShadowMapSize", 1024).toInt());
		}	

		//This gets the first camera which was found in the scene.
		Ogre::SceneManager::CameraIterator camIter = m_pActiveOgreSceneManager->getCameraIterator();
		mActiveCamera = camIter.peekNextValue();

		mMainViewport->setCamera(mActiveCamera);

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
		m_axisNode = m_pActiveOgreSceneManager->getRootSceneNode()->createChildSceneNode();

		//Create sphere representing origin
		Ogre::SceneNode* originNode = m_axisNode->createChildSceneNode();
		Ogre::Entity *originSphereEntity = m_pActiveOgreSceneManager->createEntity( "Origin Sphere", Ogre::SceneManager::PT_CUBE );
		originSphereEntity->setMaterialName("OriginMaterial");
		originNode->attachObject(originSphereEntity);
		originNode->scale(vecToUnitCube);
		originNode->scale(fOriginSize,fOriginSize,fOriginSize);

		//Create x-axis
		Ogre::SceneNode *xAxisCylinderNode = m_axisNode->createChildSceneNode();
		Ogre::Entity *xAxisCylinderEntity = m_pActiveOgreSceneManager->createEntity( "X Axis", Ogre::SceneManager::PT_CUBE );
		xAxisCylinderEntity->setMaterialName("XAxisMaterial");
		xAxisCylinderNode->attachObject(xAxisCylinderEntity);	
		xAxisCylinderNode->scale(vecToUnitCube);
		xAxisCylinderNode->scale(Ogre::Vector3(fWidth,1.0,1.0));
		xAxisCylinderNode->translate(Ogre::Vector3(fHalfWidth,0.0,0.0));

		//Create y-axis
		Ogre::SceneNode *yAxisCylinderNode = m_axisNode->createChildSceneNode();
		Ogre::Entity *yAxisCylinderEntity = m_pActiveOgreSceneManager->createEntity( "Y Axis", Ogre::SceneManager::PT_CUBE );
		yAxisCylinderEntity->setMaterialName("YAxisMaterial");
		yAxisCylinderNode->attachObject(yAxisCylinderEntity);		
		yAxisCylinderNode->scale(vecToUnitCube);
		yAxisCylinderNode->scale(Ogre::Vector3(1.0,fHeight,1.0));
		yAxisCylinderNode->translate(Ogre::Vector3(0.0,fHalfHeight,0.0));

		//Create z-axis
		Ogre::SceneNode *zAxisCylinderNode = m_axisNode->createChildSceneNode();
		Ogre::Entity *zAxisCylinderEntity = m_pActiveOgreSceneManager->createEntity( "Z Axis", Ogre::SceneManager::PT_CUBE );
		zAxisCylinderEntity->setMaterialName("ZAxisMaterial");
		zAxisCylinderNode->attachObject(zAxisCylinderEntity);
		zAxisCylinderNode->scale(vecToUnitCube);
		zAxisCylinderNode->scale(Ogre::Vector3(1.0,1.0,fDepth));
		zAxisCylinderNode->translate(Ogre::Vector3(0.0,0.0,fHalfDepth));

		//Create remainder of box		
		Ogre::ManualObject* remainingBox = m_pActiveOgreSceneManager->createManualObject("Remaining Box");
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
		if(!mMap->volumeResource.isNull())
		{		
			//Some values we'll need later.
			uint16_t regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();
			uint16_t halfRegionSideLength = regionSideLength / 2;
			uint16_t volumeWidthInRegions = mMap->volumeResource->getVolume()->getWidth() / regionSideLength;
			uint16_t volumeHeightInRegions = mMap->volumeResource->getVolume()->getHeight() / regionSideLength;
			uint16_t volumeDepthInRegions = mMap->volumeResource->getVolume()->getDepth() / regionSideLength;

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

						//The regions distance from the camera is used for prioritizing surface extraction
						Ogre::Vector3 cameraPos = mActiveCamera->getPosition();
						Ogre::Vector3 centre(centreX, centreY, centreZ);
						double distanceFromCameraSquared = (cameraPos - centre).squaredLength();

						//There's no guarentee that the MapRegion exists at this point...
						MapRegion* pMapRegion = m_volMapRegions->getVoxelAt(regionX, regionY, regionZ);

						//If the region has changed then we may need to add or remove MapRegion to/from the scene graph
						uint32_t uRegionTimeStamp = volumeChangeTracker->getLastModifiedTimeForRegion(regionX, regionY, regionZ);
						if(uRegionTimeStamp > m_volRegionTimeStamps->getVoxelAt(regionX,regionY,regionZ))
						{
							m_volRegionBeingProcessed->setVoxelAt(regionX,regionY,regionZ,true);

							//Convert to a real PolyVox::Region
							Vector3DInt16 v3dLowerCorner(firstX,firstY,firstZ);
							Vector3DInt16 v3dUpperCorner(lastX,lastY,lastZ);
							PolyVox::Region region(v3dLowerCorner, v3dUpperCorner);
							region.cropTo(volumeChangeTracker->getWrappedVolume()->getEnclosingRegion());

							//The prioirty ensures that the surfaces for regions close to the
							//camera get extracted before those which are distant from the camera.
							uint32_t uPriority = std::numeric_limits<uint32_t>::max() - static_cast<uint32_t>(distanceFromCameraSquared);

							//Extract the region
							SurfaceExtractorTaskData taskData(region, uRegionTimeStamp);
							SurfaceMeshExtractionTask* surfaceMeshExtractionTask = new SurfaceMeshExtractionTask(taskData, this);
							QObject::connect(surfaceMeshExtractionTask, SIGNAL(finished(SurfaceExtractorTaskData)), this, SLOT(uploadSurfaceExtractorResult(SurfaceExtractorTaskData)), Qt::QueuedConnection);
							QThreadPool::globalInstance()->start(surfaceMeshExtractionTask, uPriority);
							m_iNoSubmitted++;

							//Indicate that we've processed this region
							m_volRegionTimeStamps->setVoxelAt(regionX,regionY,regionZ,volumeChangeTracker->getLastModifiedTimeForRegion(regionX, regionY, regionZ));
						}
					}
				}
			}
		}		
	}

	void ThermiteGameLogic::uploadSurfaceExtractorResult(SurfaceExtractorTaskData result)
	{
		uint16_t regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();

		//Determine where it came from
		PolyVox::uint16_t regionX = result.getRegion().getLowerCorner().getX() / regionSideLength;
		PolyVox::uint16_t regionY = result.getRegion().getLowerCorner().getY() / regionSideLength;
		PolyVox::uint16_t regionZ = result.getRegion().getLowerCorner().getZ() / regionSideLength;

		uint32_t uRegionTimeStamp = volumeChangeTracker->getLastModifiedTimeForRegion(regionX, regionY, regionZ);
		if(uRegionTimeStamp > result.m_uTimeStamp)
		{
			// The volume has changed since the command to generate this mesh was issued.
			// Just ignore it, and a correct version should be along soon...
			return;
		}

		uploadSurfaceMesh(result.getSurfaceMesh(), result.getRegion());


		SurfaceMeshDecimationTask* pOldSurfaceDecimator = m_volSurfaceDecimators->getVoxelAt(regionX, regionY, regionZ);

		m_backgroundThread->removeTask(pOldSurfaceDecimator);

		SurfaceMeshDecimationTask* surfaceMeshDecimationTask = new SurfaceMeshDecimationTask(result, this);
		QObject::connect(surfaceMeshDecimationTask, SIGNAL(finished(SurfaceExtractorTaskData)), this, SLOT(uploadSurfaceDecimatorResult(SurfaceExtractorTaskData)), Qt::QueuedConnection);

		m_volSurfaceDecimators->setVoxelAt(regionX, regionY, regionZ, surfaceMeshDecimationTask);

		m_backgroundThread->addTask(surfaceMeshDecimationTask);
	}

	void ThermiteGameLogic::uploadSurfaceDecimatorResult(SurfaceExtractorTaskData result)
	{
		uint16_t regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();

		//Determine where it came from
		PolyVox::uint16_t regionX = result.getRegion().getLowerCorner().getX() / regionSideLength;
		PolyVox::uint16_t regionY = result.getRegion().getLowerCorner().getY() / regionSideLength;
		PolyVox::uint16_t regionZ = result.getRegion().getLowerCorner().getZ() / regionSideLength;

		uint32_t uRegionTimeStamp = volumeChangeTracker->getLastModifiedTimeForRegion(regionX, regionY, regionZ);
		if(uRegionTimeStamp > result.m_uTimeStamp)
		{
			// The volume has changed since the command to generate this mesh was issued.
			// Just ignore it, and a correct version should be along soon...
			return;
		}

		uploadSurfaceMesh(result.getSurfaceMesh(), result.getRegion());
	}

	void ThermiteGameLogic::uploadSurfaceMesh(POLYVOX_SHARED_PTR<SurfaceMesh> mesh, Region region)
	{
		bool bSimulatePhysics = qApp->settings()->value("Physics/SimulatePhysics", false).toBool();
		uint16_t regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();

		//Determine where it came from
		PolyVox::uint16_t regionX = region.getLowerCorner().getX() / regionSideLength;
		PolyVox::uint16_t regionY = region.getLowerCorner().getY() / regionSideLength;
		PolyVox::uint16_t regionZ = region.getLowerCorner().getZ() / regionSideLength;

		/*uint32_t uRegionTimeStamp = volumeChangeTracker->getLastModifiedTimeForRegion(regionX, regionY, regionZ);
		if(uRegionTimeStamp > result.m_uTimeStamp)
		{
			// The volume has changed since the command to generate this mesh was issued.
			// Just ignore it, and a correct version should be along soon...
			return;
		}*/

		//Create a MapRegion for that location if we don't have one already
		MapRegion* pMapRegion = m_volMapRegions->getVoxelAt(regionX, regionY, regionZ);
		if(pMapRegion == 0)
		{
			pMapRegion = new MapRegion(mMap, region.getLowerCorner());
			m_volMapRegions->setVoxelAt(regionX, regionY, regionZ, pMapRegion);
		}

		//Clear any previous geometry
		pMapRegion->removeAllSurfacePatchRenderables();

		//Get the SurfaceMesh and check it's valid
		POLYVOX_SHARED_PTR<SurfaceMesh> meshWhole = mesh;
		if((meshWhole) && (meshWhole->isEmpty() == false))
		{			
			//The SurfaceMesh needs to be broken into pieces - one for each material. Iterate over the mateials...
			for(std::map< std::string, std::set<PolyVox::uint8_t> >::iterator iter = mMap->m_mapMaterialIds.begin(); iter != mMap->m_mapMaterialIds.end(); iter++)
			{
				//Get the properties
				std::string materialName = iter->first;
				std::set<uint8_t> voxelValues = iter->second;

				//Extract the part of the InexedSurfacePatch which corresponds to that material
				POLYVOX_SHARED_PTR<SurfaceMesh> meshSubset = meshWhole->extractSubset(voxelValues);

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

		m_volRegionBeingProcessed->setVoxelAt(regionX,regionY,regionZ,false);
	}

	std::pair<bool, Ogre::Vector3> ThermiteGameLogic::getRayVolumeIntersection(const Ogre::Ray& ray)
	{
		//Initialise to failure
		std::pair<bool, Ogre::Vector3> result;
		result.first = false;
		result.second = Ogre::Vector3::ZERO;

		//Ensure the voume is valid
		PolyVox::Volume<PolyVox::uint8_t>* pVolume = mMap->volumeResource->getVolume();
		if(pVolume == 0)
		{
			return result;
		}

		Ogre::Real dist = 0.0f;
		for(int steps = 0; steps < 1000; steps++)
		{
			Ogre::Vector3 point = ray.getPoint(dist);
			PolyVox::Vector3DUint16 v3dPoint = PolyVox::Vector3DUint16(point.x + 0.5, point.y + 0.5, point.z + 0.5);
			bool inside = pVolume->getEnclosingRegion().containsPoint(static_cast<Vector3DInt16>(v3dPoint), 2);
				
			if((inside) && (pVolume->getVoxelAt(v3dPoint) != 0))
			{
				result.first = true;
				result.second = point;
				return result;
			}

			dist += 1.0f;			
		}

		return result;
	}
}