#include "DemoGameLogic.h"
#include "MainMenu.h"

#include "LogManager.h"
#include "OgreWidget.h"

#include <OgreEntity.h>
#include <OgreRenderWindow.h>
#include <OgreResourceGroupManager.h>
#include <OgreRoot.h>

namespace QtOgre
{
	DemoGameLogic::DemoGameLogic(void)
		:GameLogic()
	{
	}

	void DemoGameLogic::initialise(void)
	{
		mDemoLog = mApplication->createLog("Demo");

		mDemoLog->logMessage("A demonstration debug message", LL_DEBUG);
		mDemoLog->logMessage("A demonstration info message", LL_INFO);
		mDemoLog->logMessage("A demonstration warning message", LL_WARNING);
		mDemoLog->logMessage("A demonstration error message", LL_ERROR);

		Ogre::ResourceGroupManager::getSingleton().addResourceLocation("./media/models", "FileSystem");
		Ogre::ResourceGroupManager::getSingleton().addResourceLocation("./media/textures", "FileSystem");
		Ogre::ResourceGroupManager::getSingleton().addResourceLocation("./media/materials", "FileSystem");
		Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

		// Create the generic scene manager
		mSceneManager = Ogre::Root::getSingleton().createSceneManager(Ogre::ST_GENERIC, "GenericSceneManager");

		mCamera = mSceneManager->createCamera("Cam");

		mCamera->setPosition(10, 10, 10);
		mCamera->lookAt(0, 0, 0);
		mCamera->setNearClipDistance(1.0);
		mCamera->setFarClipDistance(1000.0);
		mCamera->setFOVy(Ogre::Radian(1.0f));

		mApplication->ogreRenderWindow()->addViewport(mCamera)->setBackgroundColour(Ogre::ColourValue::Black);

		mSceneManager->setAmbientLight( Ogre::ColourValue( 1, 1, 1 ) );

		mJaiquaEntity = mSceneManager->createEntity( "Jaiqua", "jaiqua.mesh" );
		mJaiquaNode = mSceneManager->getRootSceneNode()->createChildSceneNode( "JaiquaNode" );
		mJaiquaNode->attachObject( mJaiquaEntity );
		mJaiquaNode->scale(0.4,0.4,0.4);
		mJaiquaNode->rotate(Ogre::Vector3(0.0,1.0,0.0), Ogre::Radian(-1.57));

		mJaiquaEntity->getAnimationState("Walk")->setLoop(true);
		mJaiquaEntity->getAnimationState("Walk")->setEnabled(true);

		mRobotEntity = mSceneManager->createEntity( "Robot", "robot.mesh" );
		mRobotNode = mSceneManager->getRootSceneNode()->createChildSceneNode( "RobotNode" );
		mRobotNode->attachObject( mRobotEntity );
		mRobotNode->scale(0.1,0.1,0.1);
		mRobotNode->translate(Ogre::Vector3(3.0,0.0,0.0));
		mRobotEntity->setVisible(false);

		mRobotEntity->getAnimationState("Walk")->setLoop(true);
		mRobotEntity->getAnimationState("Walk")->setEnabled(true);

		mMainMenu = new MainMenu(qApp, qApp->mainWidget());

		mChooseMeshWidget = new ChooseMeshWidget(mJaiquaEntity, mRobotEntity, qApp->mainWidget());
		mChooseMeshWidget->setWindowOpacity(qApp->settings()->value("System/DefaultWindowOpacity", 1.0).toDouble());
		mChooseMeshWidget->move(qApp->mainWidget()->geometry().left() + qApp->mainWidget()->geometry().width() - mChooseMeshWidget->frameGeometry().width() - 10, qApp->mainWidget()->geometry().top() + 10);
		mChooseMeshWidget->show();

		mTime = new QTime;
		mTime->start();

		mIsFirstFrame = true;

		mCameraSpeed = 10.0;
	}

	void DemoGameLogic::update(void)
	{
		mLastFrameTime = mCurrentTime;
		mCurrentTime = mTime->elapsed();

		float timeElapsedInSeconds = (mCurrentTime - mLastFrameTime) / 1000.0f;

		mRobotEntity->getAnimationState("Walk")->addTime(timeElapsedInSeconds);
		mJaiquaEntity->getAnimationState("Walk")->addTime(timeElapsedInSeconds);

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

		if(!mIsFirstFrame)
		{
			QPoint mouseDelta = mCurrentMousePos - mLastFrameMousePos;
			mCamera->yaw(Ogre::Radian(-mouseDelta.x() * timeElapsedInSeconds));
			mCamera->pitch(Ogre::Radian(-mouseDelta.y() * timeElapsedInSeconds));

			int wheelDelta = mCurrentWheelPos - mLastFrameWheelPos;
			Ogre::Radian fov = mCamera->getFOVy();
			fov += Ogre::Radian(-wheelDelta * 0.001);
			fov = (std::min)(fov, Ogre::Radian(2.0f));
			fov = (std::max)(fov, Ogre::Radian(0.5f));
			mCamera->setFOVy(fov);
		}
		mLastFrameMousePos = mCurrentMousePos;
		mLastFrameWheelPos = mCurrentWheelPos;

		mIsFirstFrame = false;
	}

	void DemoGameLogic::shutdown(void)
	{
		mSceneManager->destroyEntity(mRobotEntity);
		mSceneManager->getRootSceneNode()->removeAndDestroyAllChildren();
		mSceneManager->destroyAllEntities();
		mSceneManager->destroyAllCameras();
		mSceneManager->destroyAllAnimationStates();
		mSceneManager->destroyAllAnimations();
		Ogre::Root::getSingleton().destroySceneManager(mSceneManager);
	}

	void DemoGameLogic::onKeyPress(QKeyEvent* event)
	{
		mKeyStates[event->key()] = KS_PRESSED;

		if(event->key() == Qt::Key_Escape)
		{
			//qApp->centerWidget(mMainMenu, qApp->mMainWindow);
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

	Log* DemoGameLogic::demoLog(void)
	{
		return mDemoLog;
	}
}