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

#include "Console.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "SkyBox.h"
#include "TaskProcessorThread.h"
#include "SurfaceMeshDecimationTask.h"
#include "SurfaceMeshExtractionTask.h"
#include "SurfacePatchRenderable.h"
#include "Material.h"
#include "QStringIODevice.h"
#include "TextManager.h"
#include "Vector3DClass.h"
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
#include <QUiLoader>
#include <QWaitCondition>

using namespace QtOgre;

using namespace std;

using namespace PolyVox;

namespace Thermite
{
	ThermiteGameLogic::ThermiteGameLogic(void)
		:GameLogic()

		//Scene representation
		,mCamera(0)
		,mSkyBox(0)
		//,mVolLastUploadedTimeStamps(0)
		//,mObjectList(0)
		,mCachedVolumeWidthInRegions(0)
		,mCachedVolumeHeightInRegions(0)
		,mCachedVolumeDepthInRegions(0)

		//Ogre's scene representation
		,mVolumeSceneNode(0)
		//,m_volOgreSceneNodes(0)
		,mCameraSceneNode(0)
		,mOgreCamera(0)
		,mMainViewport(0)
		,mOgreSceneManager(0)
		,m_axisNode(0)

		//Input
		,keyboard(0)
		,mouse(0)

		//Scripting support
		,scriptEngine(0)
		,m_pScriptEditorWidget(0)
		,m_bRunScript(true)
		//,mInitialiseScript(0)

		//User interface
		,mConsole(0)
		,mMainMenu(0)
		,m_pThermiteLogoMovie(0)
		,m_pThermiteLogoLabel(0)

		//Other
		,mFirstFind(false)
		,mThermiteLog(0)
		//,m_commandLineArgs(0)
		//mCurrentAppName(0)
	{
		mCamera = new Camera(this);
		keyboard = new Keyboard(this);
		mouse = new Mouse(this);	
		mSkyBox = new SkyBox(this);

		Object::mParentList = &mObjectList;
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
		mOgreCamera->setFarClipDistance(5000);
		mOgreSceneManager->setAmbientLight(Ogre::ColourValue(0.3f, 0.3f, 0.3f));

		mCameraSceneNode = mOgreSceneManager->createSceneNode("Camera Scene Node");
		mCameraSceneNode->attachObject(mOgreCamera);

		mMainViewport = mApplication->ogreRenderWindow()->addViewport(mOgreCamera);
		mMainViewport->setBackgroundColour(Ogre::ColourValue::Black);

		createAxis();

		//Set up and start the thermite logo animation. This plays while we initialise.
		//playStartupMovie();

		//Load engine built-in resources
		//addResourceDirectory("./resources/", "Thermite");
		//Ogre::ResourceGroupManager::getSingleton().initialiseResourceGroup("Thermite");
		//Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

		//Set the frame sate to be as high as possible
		mApplication->setAutoUpdateInterval(0);

		//Some Ogre related stuff we need to set up
		Ogre::Root::getSingletonPtr()->addMovableObjectFactory(new SurfacePatchRenderableFactory);
		VolumeManager* vm = new VolumeManager;
		vm->m_pProgressListener = new VolumeSerializationProgressListenerImpl();

		TextManager* sm = new TextManager;

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

		mConsole = new Console(scriptEngine, qApp->mainWidget(), Qt::Tool);
		mConsole->show();

		char* strAppName = m_commandLineArgs.getValue("appname");
		if(strAppName != 0)
		{
			loadApp(QString::fromAscii(strAppName));
		}
	}

	bool ThermiteGameLogic::loadApp(const QString& appName)
	{
		QString appDirectory("../share/thermite/apps/" + appName);

		QDir dirToTest(appDirectory);
		if(!dirToTest.exists())
		{
			QString message("Application " + appName + " does not exist. It should be found in the following location: " + appDirectory);
			mThermiteLog->logMessage(message, LL_ERROR);
			qApp->showErrorMessageBox(message);
			return false;
		}

		mCurrentAppName = appName;

		//Initialise all resources		
		addResourceDirectory("./resources/");
		addResourceDirectory(appDirectory);
		Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

		//Load the scripts
		TextResourcePtr pTextResource = TextManager::getSingletonPtr()->load("initialise.js", "General");
		mInitialiseScript = QString::fromStdString(pTextResource->getScriptData());

		TextResourcePtr pUpdateTextResource = TextManager::getSingletonPtr()->load("update.js", "General");
		m_pScriptEditorWidget->setScriptCode(QString::fromStdString(pUpdateTextResource->getScriptData()));

		//Run the initialise script
		QScriptValue result = scriptEngine->evaluate(mInitialiseScript);
		if (scriptEngine->hasUncaughtException())
		{
			int line = scriptEngine->uncaughtExceptionLineNumber();
			qCritical() << "uncaught exception at line" << line << ":" << result.toString();
		}	

		return true;
	}

	void ThermiteGameLogic::unloadApp(void)
	{
		Ogre::ResourceGroupManager::getSingleton().shutdownAll();

		mCurrentAppName = "";
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

		QListIterator<Object*> objectIter(mObjectList);
		while(objectIter.hasNext())
		{				
			Object* pObj = objectIter.next();

			Volume* volume = dynamic_cast<Volume*>(pObj);
			if(volume)
			{
				volume->updatePolyVoxGeometry(QVector3D(mOgreCamera->getPosition().x, mOgreCamera->getPosition().y, mOgreCamera->getPosition().z));
			}
		}

		if(mOgreSceneManager)
		{
			QListIterator<Object*> objectIter(mObjectList);
			while(objectIter.hasNext())
			{				
				Object* pObj = objectIter.next();
				if(pObj->isModified())
				{
					//Use the objects address to build unique names
					std::string objAddressAsString = QString::number(reinterpret_cast<qulonglong>(pObj), 16).toStdString();

					//Update the Object properties
					Ogre::SceneNode* sceneNode;
					std::string sceneNodeName(objAddressAsString + "_SceneNode");
					if(mOgreSceneManager->hasSceneNode(sceneNodeName))
					{
						sceneNode = mOgreSceneManager->getSceneNode(sceneNodeName);
					}
					else
					{
						sceneNode = mOgreSceneManager->getRootSceneNode()->createChildSceneNode(sceneNodeName);
					}

					QMatrix4x4 qtTransform = pObj->transform();
					Ogre::Matrix4 ogreTransform;
					for(int row = 0; row < 4; ++row)
					{
						Ogre::Real* rowPtr = ogreTransform[row];
						for(int col = 0; col < 4; ++col)
						{
							Ogre::Real* colPtr = rowPtr + col;
							*colPtr = qtTransform(row, col);
						}
					}

					sceneNode->setOrientation(ogreTransform.extractQuaternion());
					sceneNode->setPosition(ogreTransform.getTrans());

					QVector3D scale = pObj->size();
					sceneNode->setScale(Ogre::Vector3(scale.x(), scale.y(), scale.z()));

					sceneNode->setVisible(pObj->isVisible());

					//Update the Light properties
					Light* light = dynamic_cast<Light*>(pObj);
					if(light)
					{
						Ogre::Light* ogreLight;
						std::string lightName(objAddressAsString + "_Light");

						if(mOgreSceneManager->hasLight(lightName))
						{
							ogreLight = mOgreSceneManager->getLight(lightName);
						}
						else
						{
							ogreLight = mOgreSceneManager->createLight(lightName);
							Ogre::Entity* ogreEntity = mOgreSceneManager->createEntity(generateUID("PointLight Marker"), "sphere.mesh");
							sceneNode->attachObject(ogreLight);
							sceneNode->attachObject(ogreEntity);
						}

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

						//Note we negate the z axis as Thermite considers negative z
						//to be forwards. This means that lights will match cameras.
						QVector3D dir = -light->zAxis();
						ogreLight->setDirection(Ogre::Vector3(dir.x(), dir.y(), dir.z()));

						QColor col = light->getColour();
						ogreLight->setDiffuseColour(col.redF(), col.greenF(), col.blueF());
					}

					//Update the Entity properties
					Entity* entity = dynamic_cast<Entity*>(pObj);
					if(entity && (entity->meshName().isEmpty() == false))
					{
						Ogre::Entity* ogreEntity;
						std::string entityName(objAddressAsString + "_Entity");

						if(mOgreSceneManager->hasEntity(entityName))
						{
							ogreEntity = mOgreSceneManager->getEntity(entityName);
						}
						else
						{
							ogreEntity = mOgreSceneManager->createEntity(entityName, entity->meshName().toStdString());
							sceneNode->attachObject(ogreEntity);
						}						

						//Set a custom material if necessary
						if(entity->materialName().isEmpty() == false)
						{
							//NOTE: Might be sensible to check if this really need setting, perhaps it is slow.
							//But you can only get materials from SubEntities.
							ogreEntity->setMaterialName(entity->materialName().toStdString());
						}

						//Animation
						Ogre::AnimationStateSet* animationStateSet = ogreEntity->getAllAnimationStates();		
						if(animationStateSet && animationStateSet->hasAnimationState(entity->animationName().toStdString()))
						{
							Ogre::AnimationState* animationState = animationStateSet->getAnimationState(entity->animationName().toStdString());
							animationState->setEnabled(entity->animated());
							animationState->setLoop(entity->loopAnimation());
						}
					}				

					//Update the Volume properties
					Volume* volume = dynamic_cast<Volume*>(pObj);
					if(volume)
					{	
						//Create a scene node to attach this volume under
						if(mVolumeSceneNode == 0)
						{
							mVolumeSceneNode =  mOgreSceneManager->getRootSceneNode()->createChildSceneNode("VolumeSceneNode");
						}

						//If the size of the volume has changed then we need to start from scratch by throwing away our data and regenerating.
						if((mCachedVolumeWidthInRegions != volume->mVolumeWidthInRegions) || (mCachedVolumeHeightInRegions != volume->mVolumeHeightInRegions) || (mCachedVolumeDepthInRegions != volume->mVolumeDepthInRegions))
						{	
							deleteSceneNodeChildren(mVolumeSceneNode);

							mCachedVolumeWidthInRegions = volume->mVolumeWidthInRegions;
							mCachedVolumeHeightInRegions = volume->mVolumeHeightInRegions;
							mCachedVolumeDepthInRegions = volume->mVolumeDepthInRegions;

							m_axisNode->setScale(volume->m_pPolyVoxVolume->getWidth(), volume->m_pPolyVoxVolume->getHeight(), volume->m_pPolyVoxVolume->getDepth());
						
							uint32_t dimensions[3] = {mCachedVolumeWidthInRegions, mCachedVolumeHeightInRegions, mCachedVolumeDepthInRegions}; // Array dimensions

							//Create the arrays
							mVolLastUploadedTimeStamps.resize(dimensions);
							m_volOgreSceneNodes.resize(dimensions);

							//Clear the arrays
							std::fill(mVolLastUploadedTimeStamps.getRawData(), mVolLastUploadedTimeStamps.getRawData() + mVolLastUploadedTimeStamps.getNoOfElements(), 0);						
							std::fill(m_volOgreSceneNodes.getRawData(), m_volOgreSceneNodes.getRawData() + m_volOgreSceneNodes.getNoOfElements(), (Ogre::SceneNode*)0);
						}

						//Some values we'll need later.
						uint16_t volumeWidthInRegions = volume->mVolumeWidthInRegions;
						uint16_t volumeHeightInRegions = volume->mVolumeHeightInRegions;
						uint16_t volumeDepthInRegions = volume->mVolumeDepthInRegions;

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
										SurfaceMesh<PositionMaterial>* mesh = volume->m_volSurfaceMeshes[regionX][regionY][regionZ];
										PolyVox::Region reg = mesh->m_Region;
										uploadSurfaceMesh(*(volume->m_volSurfaceMeshes[regionX][regionY][regionZ]), reg, *volume);
									}
								}
							}
						}
					}

					pObj->setModified(false);
				}
			}
		}

		if(mOgreCamera)
		{
			QMatrix4x4 qtTransform = mCamera->transform();
			Ogre::Matrix4 ogreTransform;
			for(int row = 0; row < 4; ++row)
			{
				Ogre::Real* rowPtr = ogreTransform[row];
				for(int col = 0; col < 4; ++col)
				{
					Ogre::Real* colPtr = rowPtr + col;
					*colPtr = qtTransform(row, col);
				}
			}

			mCameraSceneNode->setOrientation(ogreTransform.extractQuaternion());
			mCameraSceneNode->setPosition(ogreTransform.getTrans());

			mOgreCamera->setFOVy(Ogre::Radian(mCamera->fieldOfView()));
		}

		if(mSkyBox && (mSkyBox->materialName().isEmpty() == false))
		{
			mOgreSceneManager->setSkyBox(true, mSkyBox->materialName().toStdString(), 2500);
		}

		mouse->setPreviousPosition(mouse->position());
		mouse->resetWheelDelta();
	}

	void ThermiteGameLogic::uploadSurfaceMesh(const SurfaceMesh<PositionMaterial>& mesh, PolyVox::Region region, Volume& volume)
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
			pOgreSceneNode = mVolumeSceneNode->createChildSceneNode(strNodeName);
			pOgreSceneNode->setPosition(Ogre::Vector3(region.getLowerCorner().getX(),region.getLowerCorner().getY(),region.getLowerCorner().getZ()));
			m_volOgreSceneNodes[regionX][regionY][regionZ] = pOgreSceneNode;
		}
		else
		{
			deleteSceneNodeChildren(pOgreSceneNode);
		}

		//Get the SurfaceMesh and check it's valid
		SurfaceMesh<PositionMaterial> meshWhole = mesh;
		if(meshWhole.isEmpty() == false)
		{			
			addSurfacePatchRenderable(volume.m_mapMaterialIds.begin()->first, meshWhole, region); ///[0] is HACK!!

			//The SurfaceMesh needs to be broken into pieces - one for each material. Iterate over the materials...
			/*for(std::map< std::string, std::set<uint8_t> >::iterator iter = volume.m_mapMaterialIds.begin(); iter != volume.m_mapMaterialIds.end(); iter++)
			{
				//Get the properties
				std::string materialName = iter->first;
				std::set<std::uint8_t> voxelValues = iter->second;

				//Extract the part of the InexedSurfacePatch which corresponds to that material
				polyvox_shared_ptr< SurfaceMesh<PositionMaterialNormal> > meshSubset = meshWhole.extractSubset(voxelValues);

				//And add it to the SceneNode
				addSurfacePatchRenderable(materialName, *meshSubset, region);
			}*/
		}		

		mVolLastUploadedTimeStamps[regionX][regionY][regionZ] = globals.timeStamp();
	}

	void ThermiteGameLogic::addSurfacePatchRenderable(std::string materialName, SurfaceMesh<PositionMaterial>& mesh, PolyVox::Region region)
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

		pSingleMaterialSurfacePatchRenderable->buildRenderOperationFrom(mesh);

		Ogre::AxisAlignedBox aabb(Ogre::Vector3(0.0f,0.0f,0.0f), Ogre::Vector3(regionSideLength, regionSideLength, regionSideLength));
		pSingleMaterialSurfacePatchRenderable->setBoundingBox(aabb);
	}

	void ThermiteGameLogic::uploadSurfaceMesh(const SurfaceMesh<PositionMaterialNormal>& mesh, PolyVox::Region region, Volume& volume)
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
			pOgreSceneNode = mVolumeSceneNode->createChildSceneNode(strNodeName);
			pOgreSceneNode->setPosition(Ogre::Vector3(region.getLowerCorner().getX(),region.getLowerCorner().getY(),region.getLowerCorner().getZ()));
			m_volOgreSceneNodes[regionX][regionY][regionZ] = pOgreSceneNode;
		}
		else
		{
			deleteSceneNodeChildren(pOgreSceneNode);
		}

		//Clear any previous geometry		
		/*Ogre::SceneNode::ObjectIterator iter =  pOgreSceneNode->getAttachedObjectIterator();
		while (iter.hasMoreElements())
		{
			Ogre::MovableObject* obj = iter.getNext();
			mOgreSceneManager->destroyMovableObject(obj);
		}
		pOgreSceneNode->detachAllObjects();*/

		//Get the SurfaceMesh and check it's valid
		SurfaceMesh<PositionMaterialNormal> meshWhole = mesh;
		if(meshWhole.isEmpty() == false)
		{			
			//The SurfaceMesh needs to be broken into pieces - one for each material. Iterate over the materials...
			for(std::map< std::string, std::set<uint8_t> >::iterator iter = volume.m_mapMaterialIds.begin(); iter != volume.m_mapMaterialIds.end(); iter++)
			{
				//Get the properties
				std::string materialName = iter->first;
				std::set<std::uint8_t> voxelValues = iter->second;

				//Extract the part of the InexedSurfacePatch which corresponds to that material
				polyvox_shared_ptr< SurfaceMesh<PositionMaterialNormal> > meshSubset = meshWhole.extractSubset(voxelValues);

				//And add it to the SceneNode
				addSurfacePatchRenderable(materialName, *meshSubset, region);
			}
		}		

		mVolLastUploadedTimeStamps[regionX][regionY][regionZ] = globals.timeStamp();
	}

	void ThermiteGameLogic::addSurfacePatchRenderable(std::string materialName, SurfaceMesh<PositionMaterialNormal>& mesh, PolyVox::Region region)
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

	void ThermiteGameLogic::createAxis(void)
	{
		//Create the main node for the axes
		m_axisNode = mOgreSceneManager->getRootSceneNode()->createChildSceneNode();

		//Create remainder of box		
		Ogre::ManualObject* axis = mOgreSceneManager->createManualObject("Axis");
		axis->begin("BaseWhiteNoLighting",Ogre::RenderOperation::OT_LINE_LIST);
		axis->position(0.0,	0.0, 0.0);	axis->colour(0.0, 0.0, 1.0);	axis->position(0.0,	0.0, 1.0);	axis->colour(0.0, 0.0, 1.0);
		axis->position(0.0,	1.0, 0.0);	axis->colour(1.0, 1.0, 1.0);	axis->position(0.0,	1.0, 1.0);	axis->colour(1.0, 1.0, 1.0);
		axis->position(1.0,	0.0, 0.0);	axis->colour(1.0, 1.0, 1.0);	axis->position(1.0,	0.0, 1.0);	axis->colour(1.0, 1.0, 1.0);
		axis->position(1.0,	1.0, 0.0);	axis->colour(1.0, 1.0, 1.0);	axis->position(1.0, 1.0, 1.0);	axis->colour(1.0, 1.0, 1.0);

		axis->position(0.0, 0.0, 0.0);	axis->colour(0.0, 1.0, 0.0);	axis->position(0.0,	1.0, 0.0);	axis->colour(0.0, 1.0, 0.0);
		axis->position(0.0,	0.0, 1.0);	axis->colour(1.0, 1.0, 1.0);	axis->position(0.0,	1.0, 1.0);	axis->colour(1.0, 1.0, 1.0);
		axis->position(1.0,	0.0, 0.0);	axis->colour(1.0, 1.0, 1.0);	axis->position(1.0,	1.0, 0.0);	axis->colour(1.0, 1.0, 1.0);
		axis->position(1.0,	0.0, 1.0);	axis->colour(1.0, 1.0, 1.0);	axis->position(1.0,	1.0, 1.0);	axis->colour(1.0, 1.0, 1.0);

		axis->position(0.0,	0.0, 0.0);	axis->colour(1.0, 0.0, 0.0);	axis->position(1.0,	0.0, 0.0);	axis->colour(1.0, 0.0, 0.0);
		axis->position(0.0,	0.0, 1.0);	axis->colour(1.0, 1.0, 1.0);	axis->position(1.0,	0.0, 1.0);	axis->colour(1.0, 1.0, 1.0);
		axis->position(0.0,	1.0, 0.0);	axis->colour(1.0, 1.0, 1.0);	axis->position(1.0,	1.0, 0.0);	axis->colour(1.0, 1.0, 1.0);
		axis->position(0.0,	1.0, 1.0);	axis->colour(1.0, 1.0, 1.0);	axis->position(1.0,	1.0, 1.0);	axis->colour(1.0, 1.0, 1.0);
		axis->end();

		//Attach the box to the node
		Ogre::SceneNode *axisNode = m_axisNode->createChildSceneNode();
		axisNode->attachObject(axis);		
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
				   //<< "qt.webkit"
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

		QScriptValue objectClass = scriptEngine->scriptValueFromQMetaObject<Object>();
		scriptEngine->globalObject().setProperty("Object", objectClass);

		QScriptValue volumeClass = scriptEngine->scriptValueFromQMetaObject<Volume>();
		scriptEngine->globalObject().setProperty("Volume", volumeClass);

		Vector3DClass *vector3dClass = new Vector3DClass(scriptEngine);
		scriptEngine->globalObject().setProperty("Vector3D", vector3dClass->constructor());

		QScriptValue globalsScriptValue = scriptEngine->newQObject(&globals);
		scriptEngine->globalObject().setProperty("globals", globalsScriptValue);

		QScriptValue keyboardScriptValue = scriptEngine->newQObject(keyboard);
		scriptEngine->globalObject().setProperty("keyboard", keyboardScriptValue);

		QScriptValue mouseScriptValue = scriptEngine->newQObject(mouse);
		scriptEngine->globalObject().setProperty("mouse", mouseScriptValue);

		QScriptValue cameraScriptValue = scriptEngine->newQObject(mCamera);
		scriptEngine->globalObject().setProperty("camera", cameraScriptValue);

		QScriptValue skyBoxScriptValue = scriptEngine->newQObject(mSkyBox);
		scriptEngine->globalObject().setProperty("skyBox", skyBoxScriptValue);

		QScriptValue Qt = scriptEngine->newQMetaObject(&staticQtMetaObject);
		Qt.setProperty("App", scriptEngine->newQObject(qApp));
		scriptEngine->globalObject().setProperty("Qt", Qt);
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

	void ThermiteGameLogic::deleteSceneNodeChildren(Ogre::SceneNode* sceneNode)
	{
		//Delete any attached objects
		Ogre::SceneNode::ObjectIterator iter =  sceneNode->getAttachedObjectIterator();
		while (iter.hasMoreElements())
		{
			//Destroy the objects (leaves dangling pointers?)
			Ogre::MovableObject* obj = iter.getNext();
			mOgreSceneManager->destroyMovableObject(obj);
		}
		//Clean up all dangling pointers.
		sceneNode->detachAllObjects();

		//Delete any child nodes
		Ogre::Node::ChildNodeIterator childNodeIter = sceneNode->getChildIterator();
		while(childNodeIter.hasMoreElements())
		{
			//A Node has to actually be a SceneNode or a Bone. We are not concerned with Bones at the moment.
			Ogre::SceneNode* childSceneNode = dynamic_cast<Ogre::SceneNode*>(childNodeIter.getNext());
			if(childSceneNode)
			{
				//Recursive call
				deleteSceneNodeChildren(childSceneNode);
			}
		}		
	}

	QWidget* ThermiteGameLogic::loadUIFile(const QString& filename)
	{
		TextResourcePtr pTextResource = TextManager::getSingletonPtr()->load(filename.toStdString(), "General");
		QString str = QString::fromStdString(pTextResource->getScriptData());
		QStringIODevice device(str);
		QUiLoader loader;
		QWidget* ui = loader.load(&device);
		return ui;
	}
}
