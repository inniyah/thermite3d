#include "ThermiteGameLogic.h"
#include "MainMenu.h"

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

namespace QtOgre
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

		mSceneManager->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED);
		//m_pOgreSceneManager->setShadowFarDistance(1000.0f);
		mSceneManager->setShadowTextureSelfShadow(true);
		mSceneManager->setShadowTextureCasterMaterial("ShadowMapCasterMaterial");
		mSceneManager->setShadowTexturePixelFormat(Ogre::PF_FLOAT32_R);
		mSceneManager->setShadowCasterRenderBackFaces(true);
		mSceneManager->setShadowTextureSize(1024);

		mMainMenu = new MainMenu(qApp, qApp->mainWidget());

		mTime = new QTime;
		mTime->start();

		mCurrentFrameNumber = 0;

		mCameraSpeed = 50.0;
		mCameraRotationalSpeed = 0.01;

		//Onto the good stuff...
		Ogre::Root::getSingletonPtr()->addMovableObjectFactory(new SurfacePatchRenderableFactory);
		VolumeManager* vm = new VolumeManager;
		mWorld = new World(Ogre::Vector3 (0,0,-98.1),Ogre::AxisAlignedBox (Ogre::Vector3 (-10000, -10000, -10000),Ogre::Vector3 (10000,  10000,  10000)), 0.1f, mSceneManager);
		mWorld->loadScene("Castle");

		//mCamera = mSceneManager->createCamera("Cam");
		Ogre::SceneManager::CameraIterator camIter = mSceneManager->getCameraIterator();
		mCamera = camIter.peekNextValue();

		mApplication->ogreRenderWindow()->addViewport(mCamera)->setBackgroundColour(Ogre::ColourValue::Black);
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

		//The fun stuff!
		mWorld->updatePolyVoxGeometry();

		if(mCurrentFrameNumber % 100 == 0)
		{
			srand(mCurrentFrameNumber);
			int iX = rand() % 150 + 50;
			int iY = rand() % 150 + 50;
			createCube(iX,iY);
		}

		
		mWorld->m_pOgreBulletWorld->stepSimulation(timeElapsedInSeconds, 10);

		++mCurrentFrameNumber;
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

	void ThermiteGameLogic::createCube(float xPos, float zPos)
	{	
		PhysicalObject* obj = new PhysicalObject(mWorld, "test", Ogre::Vector3(xPos,128.0,zPos));
		m_queueObjects.push(obj);

		while(m_queueObjects.size() > 25)
		{
			PhysicalObject* toDel = m_queueObjects.front();
			m_queueObjects.pop();
			delete toDel;
		}
	}
}