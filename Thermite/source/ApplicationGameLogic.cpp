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
		,m_bRunScript(true)
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
		mCameraRotationalSpeed = 0.1;	

		qApp->mainWidget()->setMouseTracking(true);

		mouse = new Mouse(this);
		camera = new Camera(this);

		initScriptEngine();	 

		initScriptEnvironment();

		m_pScriptEditorWidget = new ScriptEditorWidget(qApp->mainWidget());
		m_pScriptEditorWidget->show();

		QObject::connect(m_pScriptEditorWidget, SIGNAL(start(void)), this, SLOT(startScriptingEngine(void)));
		QObject::connect(m_pScriptEditorWidget, SIGNAL(stop(void)), this, SLOT(stopScriptingEngine(void)));
	}

	void ApplicationGameLogic::update(void)
	{
		//FIXME: This shold really be called at the end, so that it calls 
		//updatePolyVoxGeometry() after we've actually changed something!
		ThermiteGameLogic::update();

		if(mMap == 0)
			return;

		float distance = mCameraSpeed * mTimeElapsedInSeconds;

		/*QVector3D dir = camera->zAxis();
		QVector3D right = camera->xAxis();

		dir *= distance;
		right *= distance;

		if(keyboard.isPressed(Qt::Key_W))
		{
			//mActiveCamera->setPosition(mActiveCamera->getPosition() + mActiveCamera->getDirection() * distance);
			camera->translate(-dir.x(), -dir.y(), -dir.z());
		}
		if(keyboard.isPressed(Qt::Key_S))
		{
			//mActiveCamera->setPosition(mActiveCamera->getPosition() - mActiveCamera->getDirection() * distance);
			camera->translate(dir);
		}
		if(keyboard.isPressed(Qt::Key_A))
		{
			//mActiveCamera->setPosition(mActiveCamera->getPosition() - mActiveCamera->getRight() * distance);
			camera->translate(-right.x(), -right.y(), -right.z());
		}
		if(keyboard.isPressed(Qt::Key_D))
		{
			//mActiveCamera->setPosition(mActiveCamera->getPosition() + mActiveCamera->getRight() * distance);
			camera->translate(right);
		}
		
		if(mouse->isPressed(Qt::RightButton))
		{
			if(mCurrentFrameNumber != 0)
			{
				float mouseDeltaX = mouse->position().x() - mouse->previousPosition().x();
				camera->yaw(-mouseDeltaX * mCameraRotationalSpeed);
				//mActiveCamera->yaw(Ogre::Radian(-mouseDeltaX * mCameraRotationalSpeed));

				float mouseDeltaY = mouse->position().y() - mouse->previousPosition().y();
				camera->pitch(-mouseDeltaY * mCameraRotationalSpeed);
				//mActiveCamera->pitch(Ogre::Radian(-mouseDeltaY * mCameraRotationalSpeed));
			}
		}*/

		if(m_bRunScript)
		{
			QScriptValue result = scriptEngine->evaluate(m_pScriptEditorWidget->getScriptCode());
			if (scriptEngine->hasUncaughtException())
			{
				int line = scriptEngine->uncaughtExceptionLineNumber();
				qCritical() << "uncaught exception at line" << line << ":" << result.toString();
			}
		}

		mActiveCamera->setPosition(Ogre::Vector3(camera->position().x(), camera->position().y(), camera->position().z()));
		mActiveCamera->setOrientation(Ogre::Quaternion(camera->orientation().scalar(), camera->orientation().x(), camera->orientation().y(), camera->orientation().z()));
		mActiveCamera->setFOVy(Ogre::Radian(camera->fieldOfView()));

		//mLastFrameMousePos = mCurrentMousePos;
		//mLastFrameWheelPos = mCurrentWheelPos;

		mouse->setPreviousPosition(mouse->position());
		mouse->resetWheelDelta();
	}

	void ApplicationGameLogic::onKeyPress(QKeyEvent* event)
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

	void ApplicationGameLogic::onKeyRelease(QKeyEvent* event)
	{
		keyboard.release(event->key());
	}

	void ApplicationGameLogic::onMousePress(QMouseEvent* event)
	{
		mouse->press(event->button());
	
		//Update the mouse position as well or we get 'jumps'
		mouse->setPosition(event->pos());
		mouse->setPreviousPosition(mouse->position());
	}

	void ApplicationGameLogic::onMouseRelease(QMouseEvent* event)
	{
		mouse->release(event->button());
	}

	void ApplicationGameLogic::onMouseMove(QMouseEvent* event)
	{
		//mCurrentMousePos = event->pos();
		mouse->setPosition(event->pos());
	}

	void ApplicationGameLogic::onWheel(QWheelEvent* event)
	{
		//mCurrentWheelPos += event->delta();
		mouse->modifyWheelDelta(event->delta());
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

	void ApplicationGameLogic::initScriptEngine(void)
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

	void ApplicationGameLogic::initScriptEnvironment(void)
	{
		mGlobals = new Globals(this);

		QScriptValue lightClass = scriptEngine->scriptValueFromQMetaObject<Light>();
		scriptEngine->globalObject().setProperty("Light", lightClass);

		QScriptValue entityClass = scriptEngine->scriptValueFromQMetaObject<Entity>();
		scriptEngine->globalObject().setProperty("Entity", entityClass);

		QScriptValue globalsScriptValue = scriptEngine->newQObject(mGlobals);
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

	void ApplicationGameLogic::startScriptingEngine(void)
	{
		m_bRunScript = true;
	}

	void ApplicationGameLogic::stopScriptingEngine(void)
	{
		m_bRunScript = false;
	}
}