#include "ApplicationGameLogic.h"

#include "CannonController.h"
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
	ApplicationGameLogic::ApplicationGameLogic(void)
		:ThermiteGameLogic()
	{
	}

	void ApplicationGameLogic::initialise(void)
	{
		ThermiteGameLogic::initialise();

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
		mCameraRotationalSpeed = 0.01;		
	}

	void ApplicationGameLogic::update(void)
	{
		//FIXME: This shold really be called at the end, so that it calls 
		//updatePolyVoxGeometry() after we've actually changed something!
		ThermiteGameLogic::update();

		if(mMap == 0)
			return;

		/*mLastFrameTime = mCurrentTime;
		mCurrentTime = mTime->elapsed();

		float timeElapsedInSeconds = (mCurrentTime - mLastFrameTime) / 1000.0f;*/

		float distance = mCameraSpeed * mTimeElapsedInSeconds;

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
		}

		//The fun stuff!
		/*updatePolyVoxGeometry();
		
#ifdef ENABLE_BULLET_PHYSICS
		if((qApp->settings()->value("Physics/SimulatePhysics", false).toBool()) && (bLoadComplete))
		{
			m_pOgreBulletWorld->stepSimulation(timeElapsedInSeconds, 10);
		}
#endif //ENABLE_BULLET_PHYSICS
*/
		list<Shell*> shellsToDelete;

		for(list<Shell*>::iterator iter = m_listShells.begin(); iter != m_listShells.end(); iter++)
		{
			(*iter)->update(mTimeElapsedInSeconds);
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

		//++mCurrentFrameNumber;
	}

	void ApplicationGameLogic::onKeyPress(QKeyEvent* event)
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

	void ApplicationGameLogic::onKeyRelease(QKeyEvent* event)
	{
		mKeyStates[event->key()] = KS_RELEASED;
	}

	void ApplicationGameLogic::onMousePress(QMouseEvent* event)
	{
		mCurrentMousePos = event->pos();
		mLastFrameMousePos = mCurrentMousePos;
	}

	void ApplicationGameLogic::onMouseMove(QMouseEvent* event)
	{
		mCurrentMousePos = event->pos();
	}

	void ApplicationGameLogic::onWheel(QWheelEvent* event)
	{
		mCurrentWheelPos += event->delta();
	}

	void ApplicationGameLogic::fireCannon(void)
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

	void ApplicationGameLogic::createSphereAt(PolyVox::Vector3DFloat centre, float radius, PolyVox::uint8_t value)
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

	void ApplicationGameLogic::onLoadMapClicked(QString strMapName)
	{
		mMainMenu->hide();
		//Temporary hack until loading new map is fixed...
		mMainMenu->disableLoadButton();

		loadMap(strMapName);

		try
		{
			mTurretNode = dynamic_cast<Ogre::SceneNode*>(m_pActiveOgreSceneManager->getRootSceneNode()->getChild("chassis")->getChild("turret_main"));
			mGunNode = dynamic_cast<Ogre::SceneNode*>(m_pActiveOgreSceneManager->getRootSceneNode()->getChild("chassis")->getChild("turret_main")->getChild("gun_main"));

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

		mApplication->showFPSCounter();
	}
}