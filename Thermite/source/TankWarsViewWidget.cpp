#include "TankWarsViewWidget.h"

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
#include "VolumeManager.h"
#include "Utility.h"

#include "Application.h"
#include "LogManager.h"

#include <OgreEntity.h>
#include <OgreRenderWindow.h>
#include <OgreResourceGroupManager.h>
#include <OgreRoot.h>

#include <QDirIterator>
#include <QKeyEvent>
#include <QGlobal.h>
#include <QMouseEvent>
#include <QMovie>
#include <QMutex>
#include <QSettings>
#include <QThreadPool>
#include <QTimer>
#include <QUiLoader>
#include <QWaitCondition>

#include <qmath.h>

#include "Application.h"
#include "Log.h"

#include <OgreRenderWindow.h>
#include <OgreRoot.h>
#include <OgreStringConverter.h>

#include <QCloseEvent>

#if defined(Q_WS_X11)
#include <QX11Info>
#endif

using namespace std;
using namespace PolyVox;

namespace Thermite
{
	TankWarsViewWidget::TankWarsViewWidget(QWidget* parent, Qt::WindowFlags f)
	:ViewWidget(parent, f)
	{	
	}

	TankWarsViewWidget::~TankWarsViewWidget()
	{
	}

	void TankWarsViewWidget::initialise(void)
	{
		ViewWidget::initialise();

		//Light setup
		Object* lightObject = new Object();
		light0 = new Light(lightObject);
		lightObject->setPosition(QVector3D(64,128,255));
		lightObject->lookAt(QVector3D(128,0,128));
		lightObject->mComponent = light0;

		light0->setType(Light::DirectionalLight);
		light0->setColour(QColor(255,255,255));

		//Camera setup
		cameraNode = new Object();
		cameraSpeedInUnitsPerSecond = 100;
		cameraFocusPoint = QVector3D(128, 10, 128);
		cameraElevationAngle = 30.0;
		cameraRotationAngle = 0.0;
		cameraDistance = 100.0;

		cameraObject = new Object(cameraNode);
		Camera* camera = new Camera(cameraObject);
		cameraObject->mComponent = camera;

		//Skybox setup
		Object* skyboxObject = new Object();
		SkyBox* skyBox = new SkyBox(skyboxObject);
		skyboxObject->mComponent = skyBox;
		skyBox->setMaterialName("CraterLakeMaterial");

		//A fireball
		fireballObject = new Object();
		fireball = new Entity(fireballObject);
		fireballObject->mComponent = fireball;
		fireball->setMeshName("Icosphere7.mesh");
		fireballObject->setPosition(QVector3D(128,32,128));
		fireballObject->setSize(QVector3D(5,5,5));
		fireball->setMaterialName("FireballMaterial");
		explosionSize = 10.0;

		//Cursor
		cursorObject = new Object();
		cursor = new Entity(cursorObject);
		cursorObject->mComponent = cursor;
		cursor->setMeshName("Voxel.mesh");
		cursorObject->setSize(QVector3D(1.1,1.1,1.1));

		//Missile
		Object* missileObject = new Object();
		mMissile = new Entity(missileObject);
		missileObject->mComponent = mMissile;
		mMissile->setMeshName("missile.mesh");
		missileObject->setPosition(QVector3D(128,32,128));
		mMissile->setMaterialName("VertexColourMaterial");

		//Our main volume
		Object* volumeObject = new Object();
		volume = new Volume(volumeObject);
		volumeObject->mComponent = volume;
		volume->generateMapForTankWars();
	}

	void TankWarsViewWidget::update(void)
	{
		currentTimeInSeconds = globals.timeSinceAppStart() * 0.001f;
		timeElapsedInSeconds = currentTimeInSeconds - previousTimeInMS;
		previousTimeInMS = currentTimeInSeconds;

		//Camera rotation and zooming is allowed in all states.
		if(mouse->isPressed(Qt::RightButton))
		{
			float mouseDeltaX = mouse->position().x() - mouse->previousPosition().x();
			cameraRotationAngle += mouseDeltaX;

			float mouseDeltaY = mouse->position().y() - mouse->previousPosition().y();
			cameraElevationAngle += mouseDeltaY;

			cameraElevationAngle = qMin(cameraElevationAngle, 90.0f);
			cameraElevationAngle = qMax(cameraElevationAngle, 0.0f);
		}
		if(mouse->isPressed(Qt::LeftButton))
		{
			volume->createSphereAt(cursorObject->position(), explosionSize, 0, false);
			fireballObject->setPosition(cursorObject->position());
			explosionStartTime = currentTimeInSeconds;
		}
		
		float wheelDelta = mouse->getWheelDelta();
		cameraDistance -= wheelDelta / 12; //10 units at a time.
		cameraDistance = qMin(cameraDistance, 1000.0f);
		cameraDistance = qMax(cameraDistance, 10.0f);


		cameraNode->setOrientation(QQuaternion());	
		cameraNode->yaw(-cameraRotationAngle);
		cameraNode->pitch(-cameraElevationAngle);

		cameraNode->setPosition(cameraFocusPoint); //Not from script...

		cameraObject->setOrientation(QQuaternion());
		cameraObject->setPosition(QVector3D(0,0,cameraDistance));

		//Update the mouse cursor.
		QVector3D rayOrigin = getPickingRayOrigin(mouse->position().x(),mouse->position().y());
		QVector3D rayDir = getPickingRayDir(mouse->position().x(),mouse->position().y());
		QVector3D intersection = volume->getRayVolumeIntersection(rayOrigin, rayDir);
		QVector3D clampedIntersection = QVector3D
		(
			qRound(intersection.x()),
			qRound(intersection.y()),
			qRound(intersection.z())
		);
		cursorObject->setPosition(clampedIntersection);

		//Update the fireball
		explosionAge = currentTimeInSeconds - explosionStartTime;

		//Compute radius from volume
		float fireballVolume = explosionAge * 10000.0f;
		float rCubed = (3.0*fireballVolume) / (4.0f * 3.142f);
		float r = qPow(rCubed, 1.0f/3.0f);

		float fireballRadius = r;
		if(fireballRadius > 0.001f)
		{
			fireballObject->setSize(QVector3D(fireballRadius, fireballRadius, fireballRadius));
		}

		ViewWidget::update();
	}

	void TankWarsViewWidget::shutdown(void)
	{
		ViewWidget::shutdown();
	}

	void TankWarsViewWidget::closeEvent(QCloseEvent *event)
	{
		//We ignore this event because we wish to keep the MainWindow
		//open so that the log file can be seen duing shutdown.
		event->ignore();
		qApp->shutdown();		
	}

	void TankWarsViewWidget::keyPressEvent(QKeyEvent* event)
	{
		keyboard->press(event->key());
	}

	void TankWarsViewWidget::keyReleaseEvent(QKeyEvent* event)
	{
		keyboard->release(event->key());
	}

	void TankWarsViewWidget::mousePressEvent(QMouseEvent* event)
	{
		mouse->press(event->button());
	
		//Update the mouse position as well or we get 'jumps'
		mouse->setPosition(event->pos());
		mouse->setPreviousPosition(mouse->position());
	}

	void TankWarsViewWidget::mouseReleaseEvent(QMouseEvent* event)
	{
		mouse->release(event->button());
	}

	void TankWarsViewWidget::mouseDoubleClickEvent(QMouseEvent* event)
	{
	}

	void TankWarsViewWidget::mouseMoveEvent(QMouseEvent* event)
	{
		mouse->setPosition(event->pos());
	}

	void TankWarsViewWidget::wheelEvent(QWheelEvent* event)
	{
		mouse->modifyWheelDelta(event->delta());
	}
}
