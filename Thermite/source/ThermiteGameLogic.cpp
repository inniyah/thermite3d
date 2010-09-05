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
#include "SurfacePatchRenderable.h"
#include "MaterialDensityPair.h"
#include "ScriptManager.h"
#include "VolumeManager.h"
#include "Utility.h"

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
		,m_pOgreSceneManager(0)
		,m_bRunScript(true)
		,scriptEngine(0)
		,camera(0)
		,m_pScriptEditorWidget(0)
		,mOgreCamera(0)
		,mFirstFind(true)
	{
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
		m_pOgreSceneManager = new Ogre::DefaultSceneManager("OgreSceneManager");
		mOgreCamera = m_pOgreSceneManager->createCamera("OgreCamera");
		mOgreCamera->setFOVy(Ogre::Radian(1.0));
		mOgreCamera->setNearClipDistance(1.0);
		mOgreCamera->setFarClipDistance(1000);

		mMainViewport = mApplication->ogreRenderWindow()->addViewport(mOgreCamera);
		mMainViewport->setBackgroundColour(Ogre::ColourValue::Black);

		//Set up and start the thermite logo animation. This plays while we initialise.
		//playStartupMovie();

		//Initialise all resources
		addResourceDirectory("./resources/");
		char* strAppName = m_commandLineArgs.getValue("appname");
		if(strAppName != 0)
		{
			addResourceDirectory(QString("../share/thermite/apps/") + QString::fromAscii(strAppName));
		}
		Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

		//Set the frame sate to be as high as possible
		mApplication->setAutoUpdateInterval(0);

		//Some Ogre related stuff we need to set up
		Ogre::Root::getSingletonPtr()->addMovableObjectFactory(new SurfacePatchRenderableFactory);
		VolumeManager* vm = new VolumeManager;
		vm->m_pProgressListener = new VolumeSerializationProgressListenerImpl();

		ScriptManager* sm = new ScriptManager;

		mTime = new QTime;
		mTime->start();

		mCurrentFrameNumber = 0;		

		//The main menu
		mMainMenu = new MainMenu(qApp->mainWidget(), Qt::Tool);
		Application::centerWidget(mMainMenu, qApp->mainWidget());
		QObject::connect(mMainMenu, SIGNAL(resumeClicked(void)), mMainMenu, SLOT(hide(void)));
		QObject::connect(mMainMenu, SIGNAL(quitClicked(void)), qApp->mainWidget(), SLOT(close(void)));
		QObject::connect(mMainMenu, SIGNAL(settingsClicked(void)), mApplication, SLOT(showSettingsDialog(void)));
		QObject::connect(mMainMenu, SIGNAL(viewLogsClicked(void)), mApplication, SLOT(showLogManager(void)));
		mMainMenu->show();

		qApp->mainWidget()->setMouseTracking(true);

		m_pScriptEditorWidget = new ScriptEditorWidget(qApp->mainWidget());
		m_pScriptEditorWidget->show();		

		QObject::connect(m_pScriptEditorWidget, SIGNAL(start(void)), this, SLOT(startScriptingEngine(void)));
		QObject::connect(m_pScriptEditorWidget, SIGNAL(stop(void)), this, SLOT(stopScriptingEngine(void)));

		ScriptResourcePtr pScriptResource = ScriptManager::getSingletonPtr()->load("initialise.js", "General");
		mInitialiseScript = QString::fromStdString(pScriptResource->getScriptData());

		QScriptValue result = scriptEngine->evaluate(mInitialiseScript);
		if (scriptEngine->hasUncaughtException())
		{
			int line = scriptEngine->uncaughtExceptionLineNumber();
			qCritical() << "uncaught exception at line" << line << ":" << result.toString();
		}

		/*if(qApp->settings()->value("Debug/ShowVolumeAxes", false).toBool())
		{
			createAxis(map->m_pPolyVoxVolume->getWidth(), map->m_pPolyVoxVolume->getHeight(), map->m_pPolyVoxVolume->getDepth());
		}*/

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
	}

	void ThermiteGameLogic::update(void)
	{

		mLastFrameTime = mCurrentTime;
		mCurrentTime = globals.timeSinceAppStart();

		++mCurrentFrameNumber;

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
				Volume* volume = dynamic_cast<Volume*>(pObj);
				if(volume)
				{
					if(mFirstFind)
					{
						mFirstFind = false;
						
						volume->loadFromFile("castle.volume");						

						volume->initialise();

						if(!m_axisNode)
						{
							if(qApp->settings()->value("Debug/ShowVolumeAxes", false).toBool())
							{
								createAxis(volume->m_pPolyVoxVolume->getWidth(), volume->m_pPolyVoxVolume->getHeight(), volume->m_pPolyVoxVolume->getDepth());
							}
						}

						//mMap = new Map;
						//mMap->m_pOgreSceneManager = m_pOgreSceneManager;

						//Some values we'll need later.
						std::uint16_t regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();
						std::uint16_t halfRegionSideLength = regionSideLength / 2;
						std::uint16_t volumeWidthInRegions = volume->m_pPolyVoxVolume->getWidth() / regionSideLength;
						std::uint16_t volumeHeightInRegions = volume->m_pPolyVoxVolume->getHeight() / regionSideLength;
						std::uint16_t volumeDepthInRegions = volume->m_pPolyVoxVolume->getDepth() / regionSideLength;

						uint32_t dimensions[3] = {volumeWidthInRegions, volumeHeightInRegions, volumeDepthInRegions}; // Array dimensions
						//Create the arrays
						m_volLastUploadedTimeStamps.resize(dimensions);
						m_volOgreSceneNodes.resize(dimensions);
						//Clear the arrays
						std::fill(m_volLastUploadedTimeStamps.getRawData(), m_volLastUploadedTimeStamps.getRawData() + m_volLastUploadedTimeStamps.getNoOfElements(), 0);						
						std::fill(m_volOgreSceneNodes.getRawData(), m_volOgreSceneNodes.getRawData() + m_volOgreSceneNodes.getNoOfElements(), (Ogre::SceneNode*)0);
					}
					else
					{
						volume->updatePolyVoxGeometry(QVector3D(mOgreCamera->getPosition().x, mOgreCamera->getPosition().y, mOgreCamera->getPosition().z));

						//Some values we'll need later.
						std::uint16_t regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();
						std::uint16_t halfRegionSideLength = regionSideLength / 2;
						std::uint16_t volumeWidthInRegions = volume->m_pPolyVoxVolume->getWidth() / regionSideLength;
						std::uint16_t volumeHeightInRegions = volume->m_pPolyVoxVolume->getHeight() / regionSideLength;
						std::uint16_t volumeDepthInRegions = volume->m_pPolyVoxVolume->getDepth() / regionSideLength;

						//Iterate over each region
						for(std::uint16_t regionZ = 0; regionZ < volumeDepthInRegions; ++regionZ)
						{		
							for(std::uint16_t regionY = 0; regionY < volumeHeightInRegions; ++regionY)
							{
								for(std::uint16_t regionX = 0; regionX < volumeWidthInRegions; ++regionX)
								{
									uint32_t volLatestMeshTimeStamp = volume->m_volLatestMeshTimeStamps[regionX][regionY][regionZ];
									uint32_t volLastUploadedTimeStamp = m_volLastUploadedTimeStamps[regionX][regionY][ regionZ];
									if(volLatestMeshTimeStamp > volLastUploadedTimeStamp)
									{
										SurfaceMesh* mesh = volume->m_volSurfaceMeshes[regionX][regionY][regionZ];
										PolyVox::Region reg = mesh->m_Region;
										uploadSurfaceMesh(*(volume->m_volSurfaceMeshes[regionX][regionY][regionZ]), reg, *volume);
									}
								}
							}
						}
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

	void ThermiteGameLogic::uploadSurfaceMesh(const SurfaceMesh& mesh, PolyVox::Region region, Volume& volume)
	{
		bool bSimulatePhysics = qApp->settings()->value("Physics/SimulatePhysics", false).toBool();
		std::uint16_t regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();

		//Determine where it came from
		uint16_t regionX = region.getLowerCorner().getX() / regionSideLength;
		uint16_t regionY = region.getLowerCorner().getY() / regionSideLength;
		uint16_t regionZ = region.getLowerCorner().getZ() / regionSideLength;

		//Create a SceneNode for that location if we don't have one already
		Ogre::SceneNode* pOgreSceneNode = m_volOgreSceneNodes[regionX][regionY][regionZ];
		if(pOgreSceneNode == 0)
		{
			const std::string& strNodeName = generateUID("SN");
			pOgreSceneNode = m_pOgreSceneManager->getRootSceneNode()->createChildSceneNode(strNodeName);
			pOgreSceneNode->setPosition(Ogre::Vector3(region.getLowerCorner().getX(),region.getLowerCorner().getY(),region.getLowerCorner().getZ()));
			m_volOgreSceneNodes[regionX][regionY][regionZ] = pOgreSceneNode;
		}

		//Clear any previous geometry		
		Ogre::SceneNode::ObjectIterator iter =  pOgreSceneNode->getAttachedObjectIterator();
		while (iter.hasMoreElements())
		{
			Ogre::MovableObject* obj = iter.getNext();
			m_pOgreSceneManager->destroyMovableObject(obj);
		}
		pOgreSceneNode->detachAllObjects();

		//Get the SurfaceMesh and check it's valid
		SurfaceMesh meshWhole = mesh;
		if(meshWhole.isEmpty() == false)
		{			
			//The SurfaceMesh needs to be broken into pieces - one for each material. Iterate over the materials...
			for(std::map< std::string, std::set<uint8_t> >::iterator iter = volume.m_mapMaterialIds.begin(); iter != volume.m_mapMaterialIds.end(); iter++)
			{
				//Get the properties
				std::string materialName = iter->first;
				std::set<std::uint8_t> voxelValues = iter->second;

				//Extract the part of the InexedSurfacePatch which corresponds to that material
				polyvox_shared_ptr<SurfaceMesh> meshSubset = meshWhole.extractSubset(voxelValues);

				//And add it to the SceneNode
				addSurfacePatchRenderable(materialName, *meshSubset, region);
			}
		}

		volume.m_volRegionBeingProcessed[regionX][regionY][regionZ] = false;

		m_volLastUploadedTimeStamps[regionX][regionY][regionZ] = globals.timeStamp();
	}

	void ThermiteGameLogic::addSurfacePatchRenderable(std::string materialName, SurfaceMesh& mesh, PolyVox::Region region)
	{

		std::uint16_t regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();

		//Determine where it came from
		uint16_t regionX = region.getLowerCorner().getX() / regionSideLength;
		uint16_t regionY = region.getLowerCorner().getY() / regionSideLength;
		uint16_t regionZ = region.getLowerCorner().getZ() / regionSideLength;

		Ogre::SceneNode* pOgreSceneNode = m_volOgreSceneNodes[regionX][regionY][regionZ];

		//Single Material
		SurfacePatchRenderable* pSingleMaterialSurfacePatchRenderable;

		std::string strSprName = generateUID("SPR");
		pSingleMaterialSurfacePatchRenderable = dynamic_cast<SurfacePatchRenderable*>(m_pOgreSceneManager->createMovableObject(strSprName, SurfacePatchRenderableFactory::FACTORY_TYPE_NAME));
		pSingleMaterialSurfacePatchRenderable->setMaterial(materialName);
		pSingleMaterialSurfacePatchRenderable->setCastShadows(qApp->settings()->value("Shadows/EnableShadows", false).toBool());
		pOgreSceneNode->attachObject(pSingleMaterialSurfacePatchRenderable);
		pSingleMaterialSurfacePatchRenderable->m_v3dPos = pOgreSceneNode->getPosition();

		pSingleMaterialSurfacePatchRenderable->buildRenderOperationFrom(mesh, true);

		Ogre::AxisAlignedBox aabb(Ogre::Vector3(0.0f,0.0f,0.0f), Ogre::Vector3(regionSideLength, regionSideLength, regionSideLength));
		pSingleMaterialSurfacePatchRenderable->setBoundingBox(aabb);

		//Multi material
		SurfacePatchRenderable* pMultiMaterialSurfacePatchRenderable;

		//Create additive material
		Ogre::String strAdditiveMaterialName = materialName + "_WITH_ADDITIVE_BLENDING";
		Ogre::MaterialPtr additiveMaterial = Ogre::MaterialManager::getSingleton().getByName(strAdditiveMaterialName);
		if(additiveMaterial.isNull() == true)
		{
			Ogre::MaterialPtr originalMaterial = Ogre::MaterialManager::getSingleton().getByName(materialName);
			additiveMaterial = originalMaterial->clone(strAdditiveMaterialName);
			additiveMaterial->setSceneBlending(Ogre::SBT_ADD);
		}

		strSprName = generateUID("SPR");
		pMultiMaterialSurfacePatchRenderable = dynamic_cast<SurfacePatchRenderable*>(m_pOgreSceneManager->createMovableObject(strSprName, SurfacePatchRenderableFactory::FACTORY_TYPE_NAME));
		pMultiMaterialSurfacePatchRenderable->setMaterial(strAdditiveMaterialName);
		pMultiMaterialSurfacePatchRenderable->setCastShadows(qApp->settings()->value("Shadows/EnableShadows", false).toBool());
		pOgreSceneNode->attachObject(pMultiMaterialSurfacePatchRenderable);
		pMultiMaterialSurfacePatchRenderable->m_v3dPos = pOgreSceneNode->getPosition();

		pMultiMaterialSurfacePatchRenderable->buildRenderOperationFrom(mesh, false);

		//int regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();
		//Ogre::AxisAlignedBox aabb(Ogre::Vector3(0.0f,0.0f,0.0f), Ogre::Vector3(regionSideLength, regionSideLength, regionSideLength));
		pMultiMaterialSurfacePatchRenderable->setBoundingBox(aabb);
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
		mouse->setPosition(event->pos());
	}

	void ThermiteGameLogic::onWheel(QWheelEvent* event)
	{
		mouse->modifyWheelDelta(event->delta());
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
		QScriptValue lightClass = scriptEngine->scriptValueFromQMetaObject<Light>();
		scriptEngine->globalObject().setProperty("Light", lightClass);

		QScriptValue entityClass = scriptEngine->scriptValueFromQMetaObject<Entity>();
		scriptEngine->globalObject().setProperty("Entity", entityClass);

		QScriptValue volumeClass = scriptEngine->scriptValueFromQMetaObject<Volume>();
		scriptEngine->globalObject().setProperty("Volume", volumeClass);

		QScriptValue globalsScriptValue = scriptEngine->newQObject(&globals);
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

	void ThermiteGameLogic::playStartupMovie(void)
	{
		m_pThermiteLogoMovie = new QMovie(QString::fromUtf8(":/animations/thermite_logo.mng"));
		m_pThermiteLogoLabel = new QLabel(qApp->mainWidget(), Qt::FramelessWindowHint | Qt::Tool);
		connect(m_pThermiteLogoMovie, SIGNAL(finished(void)), this, SLOT(showLastMovieFrame(void)));
		m_pThermiteLogoLabel->setMovie(m_pThermiteLogoMovie);
		m_pThermiteLogoMovie->jumpToFrame(0);
		m_pThermiteLogoLabel->resize(m_pThermiteLogoMovie->currentImage().size());
		m_pThermiteLogoLabel->show();
		m_pThermiteLogoMovie->start();
	}

	void ThermiteGameLogic::showLastMovieFrame(void)
	{
		QTimer::singleShot(1000, this, SLOT(deleteMovie()));
	}

	void ThermiteGameLogic::deleteMovie(void)
	{
		thermiteLog()->logMessage("Deleting startup movie", LL_DEBUG);
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
}
