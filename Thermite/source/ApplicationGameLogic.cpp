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

#include "MainMenu.h"
#include "LoadMapWidget.h"

#include "LogManager.h"
#include "OgreWidget.h"
#include "SurfacePatchRenderable.h"
#include "MapManager.h"
#include "VolumeManager.h"

#include "MaterialDensityPair.h"

#include <OgreEntity.h>
#include <OgreRenderWindow.h>
#include <OgreResourceGroupManager.h>
#include <OgreRoot.h>

#include <QDirIterator>
#include <QKeyEvent>
#include <QMovie>
#include <QSettings>

using namespace QtOgre;

using namespace std;

using namespace PolyVox;

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

		qApp->mainWidget()->setMouseTracking(true);
	}

	void ApplicationGameLogic::update(void)
	{
		//FIXME: This shold really be called at the end, so that it calls 
		//updatePolyVoxGeometry() after we've actually changed something!
		ThermiteGameLogic::update();

		if(mMap == 0)
			return;

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
		
		if(mMouseButtonStates.testFlag(Qt::RightButton))
		{
			if(mCurrentFrameNumber != 0)
			{
				QPoint mouseDelta = mCurrentMousePos - mLastFrameMousePos;
				mActiveCamera->yaw(Ogre::Radian(-mouseDelta.x() * mCameraRotationalSpeed));
				mActiveCamera->pitch(Ogre::Radian(-mouseDelta.y() * mCameraRotationalSpeed));
			}
		}

		mLastFrameMousePos = mCurrentMousePos;
		mLastFrameWheelPos = mCurrentWheelPos;
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
		mLastFrameMousePos = mCurrentMousePos;

		mMouseButtonStates = event->buttons();
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

	void ApplicationGameLogic::createSphereAt(PolyVox::Vector3DFloat centre, float radius, uint8_t value, bool bPaintMode)
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

		PolyVox::Region regionToLock = PolyVox::Region(PolyVox::Vector3DInt16(firstX, firstY, firstZ), PolyVox::Vector3DInt16(lastX, lastY, lastZ));

		////////////////////////////////////////////////////////////////////////////////

		//This is ugly, but basically we are making sure that we do not modify part of the volume of the mesh is currently
		//being regenerated for that part. This is to avoid 'queing up' a whole bunch of surface exreaction commands for 
		//the same region, only to have them rejected because the time stamp has changed again since they were issued.

		//At this point it probably makes sense to pull the VolumeChangeTracker from PolyVox into Thermite and have it
		//handle these checks as well.

		//Longer term, it might be interesting to introduce a 'ModyfyVolumeCommand' which can be issued to runn on seperate threads.
		//We could then schedule these so that all the ones for a given region are processed before we issue the extract surface command
		//for that region.
		const std::uint16_t firstRegionX = regionToLock.getLowerCorner().getX() >> volumeChangeTracker->m_uRegionSideLengthPower;
		const std::uint16_t firstRegionY = regionToLock.getLowerCorner().getY() >> volumeChangeTracker->m_uRegionSideLengthPower;
		const std::uint16_t firstRegionZ = regionToLock.getLowerCorner().getZ() >> volumeChangeTracker->m_uRegionSideLengthPower;

		const std::uint16_t lastRegionX = regionToLock.getUpperCorner().getX() >> volumeChangeTracker->m_uRegionSideLengthPower;
		const std::uint16_t lastRegionY = regionToLock.getUpperCorner().getY() >> volumeChangeTracker->m_uRegionSideLengthPower;
		const std::uint16_t lastRegionZ = regionToLock.getUpperCorner().getZ() >> volumeChangeTracker->m_uRegionSideLengthPower;

		for(std::uint16_t zCt = firstRegionZ; zCt <= lastRegionZ; zCt++)
		{
			for(std::uint16_t yCt = firstRegionY; yCt <= lastRegionY; yCt++)
			{
				for(std::uint16_t xCt = firstRegionX; xCt <= lastRegionX; xCt++)
				{
					//volRegionLastModified->setVoxelAt(xCt,yCt,zCt,m_uCurrentTime);
					if(m_volRegionBeingProcessed->getVoxelAt(xCt,yCt,zCt))
					{
						return;
					}
				}
			}
		}
		////////////////////////////////////////////////////////////////////////////////

		volumeChangeTracker->lockRegion(regionToLock);
		for(int z = firstZ; z <= lastZ; ++z)
		{
			for(int y = firstY; y <= lastY; ++y)
			{
				for(int x = firstX; x <= lastX; ++x)
				{
					if((centre - PolyVox::Vector3DFloat(x,y,z)).lengthSquared() <= radiusSquared)
					{
						MaterialDensityPair44 currentValue = volumeChangeTracker->getWrappedVolume()->getVoxelAt(x,y,z);
						currentValue.setDensity(MaterialDensityPair44::getMaxDensity());
						volumeChangeTracker->setLockedVoxelAt(x,y,z,currentValue);
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

		mApplication->showFPSCounter();
	}
}