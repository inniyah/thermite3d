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

namespace QtOgre
{
	DemoGameLogic::DemoGameLogic(void)
		:GameLogic()
		,mCurrentFrameNumber(0)
	{
	}

	void DemoGameLogic::initialise(void)
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

		mCamera = mSceneManager->createCamera("Cam");

		mCamera->setPosition(128,128,256);
		mCamera->lookAt(128, 0, 128);
		mCamera->setNearClipDistance(1.0);
		mCamera->setFarClipDistance(1000.0);
		mCamera->setFOVy(Ogre::Radian(1.0f));

		mApplication->ogreRenderWindow()->addViewport(mCamera)->setBackgroundColour(Ogre::ColourValue::Black);

		mSceneManager->setAmbientLight( Ogre::ColourValue( 1, 1, 1 ) );

		mMainMenu = new MainMenu(qApp, qApp->mainWidget());

		mTime = new QTime;
		mTime->start();

		mCurrentFrameNumber = 0;

		mCameraSpeed = 50.0;
		mCameraRotationalSpeed = 0.01;

		//Onto the good stuff...
		Ogre::Root::getSingletonPtr()->addMovableObjectFactory(new SurfacePatchRenderableFactory);
		Ogre::VolumeManager* vm = new Ogre::VolumeManager;
		mWorld = new World(Ogre::Vector3 (0,0,-98.1),Ogre::AxisAlignedBox (Ogre::Vector3 (-10000, -10000, -10000),Ogre::Vector3 (10000,  10000,  10000)), 0.1f, mSceneManager);
		mWorld->loadScene("Castle");
	}

	void DemoGameLogic::update(void)
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

		//FIXME: I *really* don't know why this test is necessary. For some reason the 
		//debug version works fine, but in release mode the bullet objects don't move
		//(as is the time step was zero). But printing the value indicates it's fine.
		#if defined(_DEBUG)
			mWorld->m_pOgreBulletWorld->stepSimulation(timeElapsedInSeconds, 100);
		#else
			mWorld->m_pOgreBulletWorld->stepSimulation(0.02, 100);
		#endif

		++mCurrentFrameNumber;
	}

	void DemoGameLogic::shutdown(void)
	{
		Ogre::Root::getSingleton().destroySceneManager(mSceneManager);
	}

	void DemoGameLogic::onKeyPress(QKeyEvent* event)
	{
		mKeyStates[event->key()] = KS_PRESSED;

		if(event->key() == Qt::Key_Escape)
		{
			mMainMenu->exec();
		}
	}

	void DemoGameLogic::onKeyRelease(QKeyEvent* event)
	{
		mKeyStates[event->key()] = KS_RELEASED;
	}

	void DemoGameLogic::onMousePress(QMouseEvent* event)
	{
		mCurrentMousePos = event->pos();
		mLastFrameMousePos = mCurrentMousePos;
	}

	void DemoGameLogic::onMouseMove(QMouseEvent* event)
	{
		mCurrentMousePos = event->pos();
	}

	void DemoGameLogic::onWheel(QWheelEvent* event)
	{
		mCurrentWheelPos += event->delta();
	}

	Log* DemoGameLogic::thermiteLog(void)
	{
		return mThermiteLog;
	}

	void DemoGameLogic::addResourceDirectory(const QString& directoryName)
	{
		QDirIterator it(directoryName, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
		while (it.hasNext())
		{
			Ogre::ResourceGroupManager::getSingleton().addResourceLocation(it.next().toStdString(), "FileSystem");
		} 
	}

	void DemoGameLogic::createCube(float xPos, float zPos)
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