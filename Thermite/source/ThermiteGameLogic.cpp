#include "ThermiteGameLogic.h"
#include "MainMenu.h"
#include "Shell.h"

#include "LogManager.h"
#include "OgreWidget.h"

#include "SurfacePatchRenderable.h"
#include "VolumeManager.h"

#include <OgreEntity.h>
#include <OgreRenderWindow.h>
#include <OgreResourceGroupManager.h>
#include <OgreRoot.h>

#include <QDirIterator>
#include <QKeyEvent>
#include <QSettings>

using namespace QtOgre;

using namespace std;

namespace Thermite
{
	ThermiteGameLogic::ThermiteGameLogic(void)
		:GameLogic()
		,mCurrentFrameNumber(0)
	{
	}

	void ThermiteGameLogic::initialise(void)
	{
		mApplication->setUpdateInterval(0);

		mThermiteLog = mApplication->createLog("Thermite");
		mThermiteLog->logMessage("Initialising Thermite3D Game Engine", LL_INFO);

		#if defined(_DEBUG)
			Ogre::Root::getSingletonPtr()->loadPlugin("Plugin_CgProgramManager_d");
		#else
			Ogre::Root::getSingletonPtr()->loadPlugin("Plugin_CgProgramManager");
		#endif

		addResourceDirectory("../share/thermite/");
		Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

		// Create the generic scene manager
		mSceneManager = new Ogre::DefaultSceneManager("EngineSceneManager");

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

		mMainMenu = new MainMenu(qApp, qApp->mainWidget());		

		mTime = new QTime;
		mTime->start();

		mCurrentFrameNumber = 0;

		mCameraSpeed = 50.0;
		mCameraRotationalSpeed = 0.01;

		//Onto the good stuff...
		Ogre::Root::getSingletonPtr()->addMovableObjectFactory(new SurfacePatchRenderableFactory);
		VolumeManager* vm = new VolumeManager;
		mMap = new Map(Ogre::Vector3 (0,0,-98.1),Ogre::AxisAlignedBox (Ogre::Vector3 (-10000, -10000, -10000),Ogre::Vector3 (10000,  10000,  10000)), 0.1f, mSceneManager);
		mMap->loadScene("");

		if(qApp->settings()->value("Debug/ShowVolumeAxes", false).toBool())
		{
			mMap->createAxis(256, 256, 256);
		}

		//This gets the first camera which was found in the scene.
		Ogre::SceneManager::CameraIterator camIter = mSceneManager->getCameraIterator();
		mCamera = camIter.peekNextValue();

		mApplication->ogreRenderWindow()->addViewport(mCamera)->setBackgroundColour(Ogre::ColourValue::Black);

		mCannonNode = mSceneManager->getRootSceneNode()->createChildSceneNode("CannonSceneNode");
		mCannon = mSceneManager->createEntity("CannonEntity", "Cannon.mesh");
		mCannonNode->attachObject(mCannon);
		mCannonNode->setPosition(200.0f, 65.0f, 80.0f);
		mCannonNode->setScale(3.0,3.0,3.0);

		mCannonController = new CannonController(this, qApp->mainWidget(), Qt::Tool);
		mCannonController->show();

		mApplication->hideLogManager();
	}

	void ThermiteGameLogic::update(void)
	{
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
		float directionInDegrees = mCannonController->direction();
		float elevationInDegrees = mCannonController->elevation();
		mCannonNode->setOrientation(Ogre::Quaternion::IDENTITY);
		mCannonNode->rotate(Ogre::Vector3(0.0,1.0,0.0), Ogre::Radian(directionInDegrees / 57.0));
		mCannonNode->rotate(Ogre::Vector3(-1.0,0.0,0.0), Ogre::Radian(elevationInDegrees / 57.0)); //Elevation

		//The fun stuff!
		mMap->updatePolyVoxGeometry();
		
		if(qApp->settings()->value("Physics/SimulatePhysics", false).toBool())
		{
			mMap->m_pOgreBulletWorld->stepSimulation(timeElapsedInSeconds, 10);
		}

		list<Shell*> shellsToDelete;

		for(list<Shell*>::iterator iter = m_listShells.begin(); iter != m_listShells.end(); iter++)
		{
			(*iter)->update(timeElapsedInSeconds);
			Ogre::Vector3 shellPos = (*iter)->m_pSceneNode->getPosition();

			if(mMap->volumeResource->volume->getEnclosingRegion().containsPoint(PolyVox::Vector3DFloat(shellPos.x, shellPos.y, shellPos.z), 1.0))
			{
				if(mMap->volumeResource->volume->getVoxelAt(shellPos.x, shellPos.y, shellPos.z) != 0)
				{
					createSphereAt(PolyVox::Vector3DFloat(shellPos.x, shellPos.y, shellPos.z), 10, 0);
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

	lastX = std::min(lastX,int(mMap->volumeChangeTracker->getWrappedVolume()->getWidth()-1));
	lastY = std::min(lastY,int(mMap->volumeChangeTracker->getWrappedVolume()->getHeight()-1));
	lastZ = std::min(lastZ,int(mMap->volumeChangeTracker->getWrappedVolume()->getDepth()-1));

	mMap->volumeChangeTracker->lockRegion(PolyVox::Region(PolyVox::Vector3DInt32(firstX, firstY, firstZ), PolyVox::Vector3DInt32(lastX, lastY, lastZ)));
	for(int z = firstZ; z <= lastZ; ++z)
	{
		for(int y = firstY; y <= lastY; ++y)
		{
			for(int x = firstX; x <= lastX; ++x)
			{
				if((centre - PolyVox::Vector3DFloat(x,y,z)).lengthSquared() <= radiusSquared)
				{
					mMap->volumeChangeTracker->setLockedVoxelAt(x,y,z,value);
				}
			}
		}
	}
	mMap->volumeChangeTracker->unlockRegion();
}

	void ThermiteGameLogic::shutdown(void)
	{
		Ogre::Root::getSingleton().destroySceneManager(mSceneManager);
	}

	void ThermiteGameLogic::onKeyPress(QKeyEvent* event)
	{
		mKeyStates[event->key()] = KS_PRESSED;

		if(event->key() == Qt::Key_Escape)
		{
			mMainMenu->exec();
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
		QDirIterator it(directoryName, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
		while (it.hasNext())
		{
			Ogre::ResourceGroupManager::getSingleton().addResourceLocation(it.next().toStdString(), "FileSystem");
		} 
	}

	void ThermiteGameLogic::fireCannon(void)
	{
		Shell* shell = new Shell(mMap, Ogre::Vector3(200.0f, 65.0f, 80.0f), Ogre::Vector3(0.0f, 1.0f, 0.0f));

		//Update the shell
		float directionInDegrees = mCannonController->direction();
		float elevationInDegrees = mCannonController->elevation();
		shell->m_pSceneNode->setOrientation(Ogre::Quaternion::IDENTITY);
		shell->m_pSceneNode->rotate(Ogre::Vector3(0.0,1.0,0.0), Ogre::Radian(directionInDegrees / 57.0));
		shell->m_pSceneNode->rotate(Ogre::Vector3(-1.0,0.0,0.0), Ogre::Radian(elevationInDegrees / 57.0)); //Elevation

		shell->m_vecVelocity = shell->m_pSceneNode->getLocalAxes().GetColumn(2);
		shell->m_vecVelocity.normalise();
		shell->m_vecVelocity *= 100.0f;

		m_listShells.push_back(shell);
	}
}