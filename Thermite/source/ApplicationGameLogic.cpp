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

#include "ApplicationGameLogic.h"

#include "CannonController.h"
#include "MainMenu.h"
#include "Shell.h"
#include "LoadMapWidget.h"

#include "LogManager.h"
#include "OgreWidget.h"
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
		,mSphereBrushScale(5.0f)
		,mSphereBrushMaterial(0)
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

		//The sphere brush.
		mSphereBrush = 0;
		mSphereBrushNode = 0;

		qApp->mainWidget()->setMouseTracking(true);
	}

	void ApplicationGameLogic::update(void)
	{
		//The sphere brush.
		if((mSphereBrush == 0) && (m_pActiveOgreSceneManager != 0))
		{
			mSphereBrush = m_pActiveOgreSceneManager->createEntity("Sphere Brush", "Sphere.mesh");
			mSphereBrushNode = m_pActiveOgreSceneManager->getRootSceneNode()->createChildSceneNode("Sphere Brush Node");
			mSphereBrushNode->attachObject(mSphereBrush);
			mSphereBrushNode->setScale(10.0,10.0,10.0);

			Ogre::MaterialPtr sphereBrushMaterial = Ogre::MaterialManager::getSingleton().create("Sphere Brush Material", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
			sphereBrushMaterial->setAmbient(Ogre::ColourValue(1.0,1.0,0.0,0.5));
			sphereBrushMaterial->setDiffuse(Ogre::ColourValue(1.0,1.0,0.0,0.5));
			sphereBrushMaterial->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
			mSphereBrush->setMaterial(sphereBrushMaterial);
		}

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

		/*if(mMouseButtonStates.testFlag(Qt::LeftButton))
		{
			Vector3DFloat pos(mCurrentMousePosInWorldSpace.x, mCurrentMousePosInWorldSpace.y, mCurrentMousePosInWorldSpace.z);
			createSphereAt(pos, 10, 1, false);
		}*/

		if(mCurrentWheelPos > mLastFrameWheelPos)
		{
			mSphereBrushScale += 1.0f;
		}

		if(mCurrentWheelPos < mLastFrameWheelPos)
		{
			mSphereBrushScale -= 1.0f;
		}
		
		if(mMouseButtonStates.testFlag(Qt::RightButton))
		{
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
		}
		else
		{
			if((mSphereBrush != 0) && (m_pActiveOgreSceneManager != 0))
			{
				float actualWidth = mActiveCamera->getViewport()->getActualWidth();
				float actualHeight = mActiveCamera->getViewport()->getActualHeight();

				float fNormalisedX = mCurrentMousePos.x() / actualWidth;
				float fNormalisedY = mCurrentMousePos.y() / actualHeight;

				Ogre::Ray pickingRay = mActiveCamera->getCameraToViewportRay(fNormalisedX, fNormalisedY);
				std::pair<bool, Ogre::Vector3> pickingResult = getRayVolumeIntersection(pickingRay);
				if(pickingResult.first)
				{
					mSphereBrushNode->setVisible(true);
					mCurrentMousePosInWorldSpace = pickingResult.second;
				}
				else
				{
					mSphereBrushNode->setVisible(false);
				}
			}
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
					createSphereAt(PolyVox::Vector3DFloat(shellPos.x, shellPos.y, shellPos.z), 50, 0, false);
					shellsToDelete.push_back(*iter);
				}
			}
		}

		for(list<Shell*>::iterator iter = shellsToDelete.begin(); iter != shellsToDelete.end(); iter++)
		{
			m_listShells.remove(*iter);
			delete (*iter);
		}

		//Update the brush position
		mSphereBrushNode->setPosition(mCurrentMousePosInWorldSpace);
		mSphereBrushNode->setScale(mSphereBrushScale, mSphereBrushScale, mSphereBrushScale);
	}

	void ApplicationGameLogic::onKeyPress(QKeyEvent* event)
	{
		mKeyStates[event->key()] = KS_PRESSED;

		if((event->key() >= Qt::Key_0) && (event->key() <= Qt::Key_9))
		{
			//Converts key code to correct number
			mSphereBrushMaterial = event->key() - Qt::Key_0;
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

	void ApplicationGameLogic::onKeyRelease(QKeyEvent* event)
	{
		mKeyStates[event->key()] = KS_RELEASED;
	}

	void ApplicationGameLogic::onMousePress(QMouseEvent* event)
	{
		//mCurrentMousePos = event->pos();
		mLastFrameMousePos = mCurrentMousePos;

		mMouseButtonStates = event->buttons();

		if(event->button() == Qt::LeftButton)
		{
			Vector3DFloat pos(mCurrentMousePosInWorldSpace.x, mCurrentMousePosInWorldSpace.y, mCurrentMousePosInWorldSpace.z);
			createSphereAt(pos, mSphereBrushScale, mSphereBrushMaterial, false);
		}
	}

	void ApplicationGameLogic::onMouseRelease(QMouseEvent* event)
	{
		mLastFrameMousePos = mCurrentMousePos;

		mMouseButtonStates = event->buttons();
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

	void ApplicationGameLogic::createSphereAt(PolyVox::Vector3DFloat centre, float radius, PolyVox::uint8_t value, bool bPaintMode)
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
						PolyVox::uint8_t currentValue = volumeChangeTracker->getWrappedVolume()->getVoxelAt(x,y,z);
						if(currentValue != value)
						{
							if(bPaintMode)
							{
								if(currentValue != 0)
								{
									volumeChangeTracker->setLockedVoxelAt(x,y,z,value);
								}
							}
							else
							{
								volumeChangeTracker->setLockedVoxelAt(x,y,z,value);
							}
						}
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