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

#include "Keyboard.h"
#include "Mouse.h"
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

using namespace QtOgre;

using namespace std;

using namespace PolyVox;

namespace Thermite
{
	ThermiteGameLogic::ThermiteGameLogic(void)
		:GameLogic()
		,mOgreSceneManager(0)
		,m_bRunScript(true)
		,scriptEngine(0)
		,mCamera(0)
		,m_pScriptEditorWidget(0)
		,mOgreCamera(0)
		,mFirstFind(true)
		,mPointLightMarkerNode(0)
		,m_axisNode(0)
		,keyboard(0)
	{
		mCamera = new Camera(this);
		keyboard = new Keyboard(this);
		mouse = new Mouse(this);		
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
		mOgreSceneManager = new Ogre::DefaultSceneManager("OgreSceneManager");
		mOgreCamera = mOgreSceneManager->createCamera("OgreCamera");
		mOgreCamera->setFOVy(Ogre::Radian(1.0));
		mOgreCamera->setNearClipDistance(1.0);
		mOgreCamera->setFarClipDistance(1000);
		mOgreSceneManager->setAmbientLight(Ogre::ColourValue(0.3f, 0.3f, 0.3f));

		mMainViewport = mApplication->ogreRenderWindow()->addViewport(mOgreCamera);
		mMainViewport->setBackgroundColour(Ogre::ColourValue::Black);

		//Set up and start the thermite logo animation. This plays while we initialise.
		//playStartupMovie();

		//Load engine built-in resources
		addResourceDirectory("./resources/");

		//Set the frame sate to be as high as possible
		mApplication->setAutoUpdateInterval(0);

		//Some Ogre related stuff we need to set up
		Ogre::Root::getSingletonPtr()->addMovableObjectFactory(new SurfacePatchRenderableFactory);
		VolumeManager* vm = new VolumeManager;
		vm->m_pProgressListener = new VolumeSerializationProgressListenerImpl();

		ScriptManager* sm = new ScriptManager;

		//The main menu
		mMainMenu = new MainMenu(qApp->mainWidget(), Qt::Tool);
		Application::centerWidget(mMainMenu, qApp->mainWidget());
		QObject::connect(mMainMenu, SIGNAL(resumeClicked(void)), mMainMenu, SLOT(hide(void)));
		QObject::connect(mMainMenu, SIGNAL(quitClicked(void)), qApp->mainWidget(), SLOT(close(void)));
		QObject::connect(mMainMenu, SIGNAL(settingsClicked(void)), mApplication, SLOT(showSettingsDialog(void)));
		QObject::connect(mMainMenu, SIGNAL(viewLogsClicked(void)), mApplication, SLOT(showLogManager(void)));
		//mMainMenu->show();

		qApp->mainWidget()->setMouseTracking(true);

		m_pScriptEditorWidget = new ScriptEditorWidget(qApp->mainWidget());
		//m_pScriptEditorWidget->show();		

		QObject::connect(m_pScriptEditorWidget, SIGNAL(start(void)), this, SLOT(startScriptingEngine(void)));
		QObject::connect(m_pScriptEditorWidget, SIGNAL(stop(void)), this, SLOT(stopScriptingEngine(void)));

		mPointLightMarkerNode = mOgreSceneManager->getRootSceneNode()->createChildSceneNode();

		char* strAppName = m_commandLineArgs.getValue("appname");
		if(strAppName != 0)
		{
			loadApp(QString::fromAscii(strAppName));
		}
	}

	bool ThermiteGameLogic::loadApp(const QString& appName)
	{
		//Initialise all resources		
		addResourceDirectory(QString("../share/thermite/apps/") + appName);
		Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

		//Load the scripts
		ScriptResourcePtr pScriptResource = ScriptManager::getSingletonPtr()->load("initialise.js", "General");
		mInitialiseScript = QString::fromStdString(pScriptResource->getScriptData());

		ScriptResourcePtr pUpdateScriptResource = ScriptManager::getSingletonPtr()->load("update.js", "General");
		m_pScriptEditorWidget->setScriptCode(QString::fromStdString(pUpdateScriptResource->getScriptData()));

		//Run the initialise script
		QScriptValue result = scriptEngine->evaluate(mInitialiseScript);
		if (scriptEngine->hasUncaughtException())
		{
			int line = scriptEngine->uncaughtExceptionLineNumber();
			qCritical() << "uncaught exception at line" << line << ":" << result.toString();
		}	

		return true;
	}

	void ThermiteGameLogic::update(void)
	{
		if(m_bRunScript)
		{
			QScriptValue result = scriptEngine->evaluate(m_pScriptEditorWidget->getScriptCode());
			if (scriptEngine->hasUncaughtException())
			{
				int line = scriptEngine->uncaughtExceptionLineNumber();
				qCritical() << "uncaught exception at line" << line << ":" << result.toString();
			}
		}

		if(mOgreSceneManager)
		{
			mOgreSceneManager->destroyAllLights();
			mPointLightMarkerNode->removeAndDestroyAllChildren();

			//mPointLightMarkerNode->detachAllObjects();
			QHashIterator<QString, QObject*> objectIter(mObjectStore);
			while(objectIter.hasNext())
			{
				objectIter.next();
				QObject* pObj = objectIter.value();

				Light* light = dynamic_cast<Light*>(pObj);
				if(light)
				{
					//light->setProperty("type", "DirectionalLight");
					//light->setType(DirectionalLight);

					Ogre::Light* ogreLight = mOgreSceneManager->createLight(objectIter.key().toStdString());
					switch(light->getType())
					{
					case Light::PointLight:
						ogreLight->setType(Ogre::Light::LT_POINT);
						break;
					case Light::DirectionalLight:
						ogreLight->setType(Ogre::Light::LT_DIRECTIONAL);
						break;
					case Light::SpotLight:
						ogreLight->setType(Ogre::Light::LT_SPOTLIGHT);
						break;
					}

					QVector3D pos = light->position();
					ogreLight->setPosition(Ogre::Vector3(pos.x(), pos.y(), pos.z()));

					//Note we negate the z axis as Thermite considers negative z
					//to be forwards. This means that lights will match cameras.
					QVector3D dir = -light->zAxis();
					ogreLight->setDirection(Ogre::Vector3(dir.x(), dir.y(), dir.z()));

					QColor col = light->getColour();
					ogreLight->setDiffuseColour(col.redF(), col.greenF(), col.blueF());

					//And create the marker
					Ogre::SceneNode* sceneNode = mPointLightMarkerNode->createChildSceneNode();
					Ogre::Entity* ogreEntity = mOgreSceneManager->createEntity(generateUID("PointLight Marker"), "sphere.mesh");
					sceneNode->attachObject(ogreEntity);
					sceneNode->setPosition(Ogre::Vector3(pos.x(), pos.y(), pos.z()));
				}

				Entity* entity = dynamic_cast<Entity*>(pObj);
				if(entity)
				{
					Ogre::Entity* ogreEntity;
					Ogre::SceneNode* sceneNode;

					if(mOgreSceneManager->hasEntity(objectIter.key().toStdString()))
					{
						ogreEntity = mOgreSceneManager->getEntity(objectIter.key().toStdString());
						sceneNode = dynamic_cast<Ogre::SceneNode*>(ogreEntity->getParentNode());
					}
					else
					{
						sceneNode = mOgreSceneManager->getRootSceneNode()->createChildSceneNode();
						ogreEntity = mOgreSceneManager->createEntity(objectIter.key().toStdString(), entity->meshName().toStdString());
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

						volume->initialise();						
						volume->loadFromFile("castle.volume");	

						//Some values we'll need later.
						uint16_t volumeWidthInRegions = volume->mVolumeWidthInRegions;
						uint16_t volumeHeightInRegions = volume->mVolumeHeightInRegions;
						uint16_t volumeDepthInRegions = volume->mVolumeDepthInRegions;

						if(!m_axisNode)
						{
							//if(qApp->settings()->value("Debug/ShowVolumeAxes", false).toBool())
							{
								createAxis(volume->m_pPolyVoxVolume->getWidth(), volume->m_pPolyVoxVolume->getHeight(), volume->m_pPolyVoxVolume->getDepth());
							}
						}						

						uint32_t dimensions[3] = {volumeWidthInRegions, volumeHeightInRegions, volumeDepthInRegions}; // Array dimensions
						//Create the arrays
						mVolLastUploadedTimeStamps.resize(dimensions);
						m_volOgreSceneNodes.resize(dimensions);
						//Clear the arrays
						std::fill(mVolLastUploadedTimeStamps.getRawData(), mVolLastUploadedTimeStamps.getRawData() + mVolLastUploadedTimeStamps.getNoOfElements(), 0);						
						std::fill(m_volOgreSceneNodes.getRawData(), m_volOgreSceneNodes.getRawData() + m_volOgreSceneNodes.getNoOfElements(), (Ogre::SceneNode*)0);
					}
					else
					{
						//Some values we'll need later.
						uint16_t volumeWidthInRegions = volume->mVolumeWidthInRegions;
						uint16_t volumeHeightInRegions = volume->mVolumeHeightInRegions;
						uint16_t volumeDepthInRegions = volume->mVolumeDepthInRegions;

						volume->updatePolyVoxGeometry(QVector3D(mOgreCamera->getPosition().x, mOgreCamera->getPosition().y, mOgreCamera->getPosition().z));

						//Iterate over each region
						for(std::uint16_t regionZ = 0; regionZ < volumeDepthInRegions; ++regionZ)
						{		
							for(std::uint16_t regionY = 0; regionY < volumeHeightInRegions; ++regionY)
							{
								for(std::uint16_t regionX = 0; regionX < volumeWidthInRegions; ++regionX)
								{
									uint32_t volExtractionFinsishedTimeStamp = volume->mExtractionFinishedArray[regionX][regionY][regionZ];
									uint32_t volLastUploadedTimeStamp = mVolLastUploadedTimeStamps[regionX][regionY][ regionZ];
									if(volExtractionFinsishedTimeStamp > volLastUploadedTimeStamp)
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
			mOgreCamera->setPosition(Ogre::Vector3(mCamera->position().x(), mCamera->position().y(), mCamera->position().z()));
			mOgreCamera->setOrientation(Ogre::Quaternion(mCamera->orientation().scalar(), mCamera->orientation().x(), mCamera->orientation().y(), mCamera->orientation().z()));
			mOgreCamera->setFOVy(Ogre::Radian(mCamera->fieldOfView()));
		}

		mouse->setPreviousPosition(mouse->position());
		mouse->resetWheelDelta();
	}

	void ThermiteGameLogic::uploadSurfaceMesh(const SurfaceMesh& mesh, PolyVox::Region region, Volume& volume)
	{
		bool bSimulatePhysics = qApp->settings()->value("Physics/SimulatePhysics", false).toBool();

		//Determine where it came from
		uint16_t regionX = region.getLowerCorner().getX() / volume.mRegionSideLength;
		uint16_t regionY = region.getLowerCorner().getY() / volume.mRegionSideLength;
		uint16_t regionZ = region.getLowerCorner().getZ() / volume.mRegionSideLength;

		//Create a SceneNode for that location if we don't have one already
		Ogre::SceneNode* pOgreSceneNode = m_volOgreSceneNodes[regionX][regionY][regionZ];
		if(pOgreSceneNode == 0)
		{
			const std::string& strNodeName = generateUID("SN");
			pOgreSceneNode = mOgreSceneManager->getRootSceneNode()->createChildSceneNode(strNodeName);
			pOgreSceneNode->setPosition(Ogre::Vector3(region.getLowerCorner().getX(),region.getLowerCorner().getY(),region.getLowerCorner().getZ()));
			m_volOgreSceneNodes[regionX][regionY][regionZ] = pOgreSceneNode;
		}

		//Clear any previous geometry		
		Ogre::SceneNode::ObjectIterator iter =  pOgreSceneNode->getAttachedObjectIterator();
		while (iter.hasMoreElements())
		{
			Ogre::MovableObject* obj = iter.getNext();
			mOgreSceneManager->destroyMovableObject(obj);
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

		mVolLastUploadedTimeStamps[regionX][regionY][regionZ] = globals.timeStamp();
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
		pSingleMaterialSurfacePatchRenderable = dynamic_cast<SurfacePatchRenderable*>(mOgreSceneManager->createMovableObject(strSprName, SurfacePatchRenderableFactory::FACTORY_TYPE_NAME));
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
		pMultiMaterialSurfacePatchRenderable = dynamic_cast<SurfacePatchRenderable*>(mOgreSceneManager->createMovableObject(strSprName, SurfacePatchRenderableFactory::FACTORY_TYPE_NAME));
		pMultiMaterialSurfacePatchRenderable->setMaterial(strAdditiveMaterialName);
		pMultiMaterialSurfacePatchRenderable->setCastShadows(qApp->settings()->value("Shadows/EnableShadows", false).toBool());
		pOgreSceneNode->attachObject(pMultiMaterialSurfacePatchRenderable);
		pMultiMaterialSurfacePatchRenderable->m_v3dPos = pOgreSceneNode->getPosition();

		pMultiMaterialSurfacePatchRenderable->buildRenderOperationFrom(mesh, false);

		pMultiMaterialSurfacePatchRenderable->setBoundingBox(aabb);
	}

	void ThermiteGameLogic::onKeyPress(QKeyEvent* event)
	{
		keyboard->press(event->key());

		if(event->key() == Qt::Key_F1)
		{
			qApp->showLogManager();
		}

		if(event->key() == Qt::Key_F2)
		{
			m_pScriptEditorWidget->show();
		}

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
		keyboard->release(event->key());
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
		Ogre::Root::getSingleton().destroySceneManager(mOgreSceneManager);
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
		m_axisNode = mOgreSceneManager->getRootSceneNode()->createChildSceneNode();

		//Create sphere representing origin
		Ogre::SceneNode* originNode = m_axisNode->createChildSceneNode();
		Ogre::Entity *originSphereEntity = mOgreSceneManager->createEntity( "Origin Sphere", Ogre::SceneManager::PT_CUBE );
		originSphereEntity->setMaterialName("OriginMaterial");
		originNode->attachObject(originSphereEntity);
		originNode->scale(vecToUnitCube);
		originNode->scale(fOriginSize,fOriginSize,fOriginSize);

		//Create x-axis
		Ogre::SceneNode *xAxisCylinderNode = m_axisNode->createChildSceneNode();
		Ogre::Entity *xAxisCylinderEntity = mOgreSceneManager->createEntity( "X Axis", Ogre::SceneManager::PT_CUBE );
		xAxisCylinderEntity->setMaterialName("RedMaterial");
		xAxisCylinderNode->attachObject(xAxisCylinderEntity);	
		xAxisCylinderNode->scale(vecToUnitCube);
		xAxisCylinderNode->scale(Ogre::Vector3(fWidth,1.0,1.0));
		xAxisCylinderNode->translate(Ogre::Vector3(fHalfWidth,0.0,0.0));

		//Create y-axis
		Ogre::SceneNode *yAxisCylinderNode = m_axisNode->createChildSceneNode();
		Ogre::Entity *yAxisCylinderEntity = mOgreSceneManager->createEntity( "Y Axis", Ogre::SceneManager::PT_CUBE );
		yAxisCylinderEntity->setMaterialName("GreenMaterial");
		yAxisCylinderNode->attachObject(yAxisCylinderEntity);		
		yAxisCylinderNode->scale(vecToUnitCube);
		yAxisCylinderNode->scale(Ogre::Vector3(1.0,fHeight,1.0));
		yAxisCylinderNode->translate(Ogre::Vector3(0.0,fHalfHeight,0.0));

		//Create z-axis
		Ogre::SceneNode *zAxisCylinderNode = m_axisNode->createChildSceneNode();
		Ogre::Entity *zAxisCylinderEntity = mOgreSceneManager->createEntity( "Z Axis", Ogre::SceneManager::PT_CUBE );
		zAxisCylinderEntity->setMaterialName("BlueMaterial");
		zAxisCylinderNode->attachObject(zAxisCylinderEntity);
		zAxisCylinderNode->scale(vecToUnitCube);
		zAxisCylinderNode->scale(Ogre::Vector3(1.0,1.0,fDepth));
		zAxisCylinderNode->translate(Ogre::Vector3(0.0,0.0,fHalfDepth));

		//Create remainder of box		
		Ogre::ManualObject* remainingBox = mOgreSceneManager->createManualObject("Remaining Box");
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
		QScriptValue thermiteGameLogicClass = scriptEngine->newQObject(this);
		scriptEngine->globalObject().setProperty("ThermiteGameLogic", thermiteGameLogicClass);

		QScriptValue lightClass = scriptEngine->scriptValueFromQMetaObject<Light>();
		scriptEngine->globalObject().setProperty("Light", lightClass);

		QScriptValue entityClass = scriptEngine->scriptValueFromQMetaObject<Entity>();
		scriptEngine->globalObject().setProperty("Entity", entityClass);

		QScriptValue volumeClass = scriptEngine->scriptValueFromQMetaObject<Volume>();
		scriptEngine->globalObject().setProperty("Volume", volumeClass);

		QScriptValue globalsScriptValue = scriptEngine->newQObject(&globals);
		scriptEngine->globalObject().setProperty("globals", globalsScriptValue);

		QScriptValue keyboardScriptValue = scriptEngine->newQObject(keyboard);
		scriptEngine->globalObject().setProperty("keyboard", keyboardScriptValue);

		QScriptValue mouseScriptValue = scriptEngine->newQObject(mouse);
		scriptEngine->globalObject().setProperty("mouse", mouseScriptValue);

		QScriptValue cameraScriptValue = scriptEngine->newQObject(mCamera);
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

	QVector3D ThermiteGameLogic::getPickingRayOrigin(int x, int y)
	{
		float actualWidth = mOgreCamera->getViewport()->getActualWidth();
		float actualHeight = mOgreCamera->getViewport()->getActualHeight();

		float fNormalisedX = x / actualWidth;
		float fNormalisedY = y / actualHeight;

		Ogre::Ray pickingRay = mOgreCamera->getCameraToViewportRay(fNormalisedX, fNormalisedY);

		return QVector3D(pickingRay.getOrigin().x, pickingRay.getOrigin().y, pickingRay.getOrigin().z);
	}

	QVector3D ThermiteGameLogic::getPickingRayDir(int x, int y)
	{
		float actualWidth = mOgreCamera->getViewport()->getActualWidth();
		float actualHeight = mOgreCamera->getViewport()->getActualHeight();

		float fNormalisedX = x / actualWidth;
		float fNormalisedY = y / actualHeight;

		Ogre::Ray pickingRay = mOgreCamera->getCameraToViewportRay(fNormalisedX, fNormalisedY);

		return QVector3D(pickingRay.getDirection().x, pickingRay.getDirection().y, pickingRay.getDirection().z);
	}
}
