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

#include "Perlin.h"

#include <OgreEntity.h>
#include <OgreRenderWindow.h>
#include <OgreResourceGroupManager.h>
#include <OgreRoot.h>

#include <QDirIterator>
#include <QKeyEvent>
#include <qglobal.h>
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
		lightObject->setPosition(QVector3D(0,128,128));
		lightObject->lookAt(QVector3D(128,0,128));

		light0 = new Light(lightObject);
		light0->setType(Light::DirectionalLight);
		light0->setColour(QColor(255,255,255));

		//Camera setup
		cameraNode = new Object();
		cameraSpeedInUnitsPerSecond = 100;
		cameraFocusPoint = QVector3D(128, 10, 128);
		cameraElevationAngle = 30.0;
		cameraRotationAngle = 0.0;
		cameraDistance = 100.0;

		//cameraObject = new Object(cameraNode);
		//Camera* camera = new Camera();
		mCamera->setParent(cameraNode);

		//Skybox setup
		Object* skyboxObject = new Object();
		SkyBox* skyBox = new SkyBox(skyboxObject);
		skyBox->setMaterialName("CraterLakeMaterial");

		//A fireball
		fireballObject = new Object();
		fireball = new Entity(fireballObject);
		fireball->setMeshName("Icosphere7.mesh");
		fireballObject->setPosition(QVector3D(128,32,128));
		fireballObject->setSize(QVector3D(5,5,5));
		fireball->setMaterialName("FireballMaterial");
		explosionSize = 10.0;

		//Cursor
		cursorObject = new Object();
		cursor = new Entity(cursorObject);
		cursor->setMeshName("Voxel.mesh");
		cursorObject->setSize(QVector3D(1.1,1.1,1.1));

		//Missile
		Object* missileObject = new Object();
		mMissile = new Entity(missileObject);
		mMissile->setMeshName("missile.mesh");
		missileObject->setPosition(QVector3D(128,32,128));
		mMissile->setMaterialName("VertexColourMaterial");

		//Our main volume
		//Object* volumeObject = new Object();
		mVolume = new Volume(256, 64, 256);
		generateMapForTankWars(mVolume);
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
			mVolume->createSphereAt(cursorObject->position(), explosionSize, 0, false);
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

		mCamera->setOrientation(QQuaternion());
		mCamera->setPosition(QVector3D(0,0,cameraDistance));

		//Update the mouse cursor.
		QVector3D rayOrigin = getPickingRayOrigin(mouse->position().x(),mouse->position().y());
		QVector3D rayDir = getPickingRayDir(mouse->position().x(),mouse->position().y());
		QVector3D intersection = mVolume->getRayVolumeIntersection(rayOrigin, rayDir);
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

	void TankWarsViewWidget::generateMapForTankWars(Volume* volume)
	{
		generateRockyMapForTankWars(volume);
	}

	void TankWarsViewWidget::generateHillyMapForTankWars(Volume* volume)
	{
		const int mapWidth = 256;
		const int mapHeight = 32;
		const int mapDepth = 256;

		int volumeHeight = 64;

		PolyVox::Volume<PolyVox::Material8>* pPolyVoxVolume = volume->m_pPolyVoxVolume;

		//Create a grid of Perlin noise values
		Perlin perlin(2,4,1,234);
		float perlinValues[mapWidth][mapDepth];
		float minPerlinValue = 1000.0f;
		float maxPerlinValue = -1000.0f;
		for(int z = 0; z < mapDepth; z++)
		{
			for(int x = 0; x < mapWidth; x++) 
			{
				perlinValues[x][z] = perlin.Get(x /static_cast<float>(mapWidth-1), z / static_cast<float>(mapDepth-1));
				minPerlinValue = std::min(minPerlinValue, perlinValues[x][z]);
				maxPerlinValue = std::max(maxPerlinValue, perlinValues[x][z]);
			}
		}

		//Normalise values so that th smallest is 0.0 and the biggest is 1.0
		float range = maxPerlinValue - minPerlinValue;
		for(int z = 0; z < mapDepth; z++)
		{
			for(int x = 0; x < mapWidth; x++) 
			{
				perlinValues[x][z] = (perlinValues[x][z] - minPerlinValue) / range;
			}
		}

		//Introduce a flat area into the map. This code saves the top and bottom parts and collapses the rest.
		for(int z = 0; z < mapDepth; z++)
		{
			for(int x = 0; x < mapWidth; x++) 
			{				
				float flatAreaSize = 1.0; //0.0 gives no flat area, larger number give increasing flat area.

				perlinValues[x][z] = perlinValues[x][z] * (flatAreaSize + 1.0f);

				float desiredGroundHeight = 0.25f;
				if(perlinValues[x][z] > desiredGroundHeight)
				{
					perlinValues[x][z] = std::max(desiredGroundHeight, perlinValues[x][z] - flatAreaSize);
				}				
			}
		}

		//Copy the data into the volume
		for(int z = 0; z < mapDepth; z++)
		{
			for(int x = 0; x < mapWidth; x++) 
			{							
				int terrainHeight = perlinValues[x][z] * (mapHeight-1);

				for(int y = 0; y < mapHeight; y++)
				{
					Material8 voxel;
					if(y < terrainHeight)
					{
						voxel.setMaterial(130);
						voxel.setDensity(Material8::getMaxDensity());
					}
					else if(y == terrainHeight)
					{
						voxel.setMaterial(60);
						voxel.setDensity(Material8::getMaxDensity());
					}
					else
					{
						voxel.setMaterial(0);
						voxel.setDensity(Material8::getMinDensity());
					}

					pPolyVoxVolume->setVoxelAt(x,y,z,voxel);
				}
			}
		}

		return;
	}

	void TankWarsViewWidget::generateRockyMapForTankWars(Volume* volume)
	{
		const int mapWidth = 256;
		const int mapHeight = 32;
		const int mapDepth = 256;

		int volumeHeight = 64;

		PolyVox::Volume<PolyVox::Material8>* pPolyVoxVolume = volume->m_pPolyVoxVolume;

		Perlin perlin(2,4,1,234);

		for(int z = 0; z < mapDepth; z++)
		{
			for(int y = 0; y < mapHeight; y++)
			{
				for(int x = 0; x < mapWidth; x++) 
				{							
					//float perlinVal = perlin.Get3D(x /static_cast<float>(mapWidth-1), (y/8) / static_cast<float>(mapHeight-1), z / static_cast<float>(mapDepth-1));

					float perlinVal = perlin.Get3D(x /static_cast<float>(mapWidth-1), (y/8.0f) / static_cast<float>(mapHeight-1), z / static_cast<float>(mapDepth-1));

					float height = y / static_cast<float>(mapHeight);

					height *= height;
					height *= height;
					height *= height;

					Material8 voxel;
					if(perlinVal + height < 0.0f)
					{
						voxel.setMaterial(239);
						voxel.setDensity(Material8::getMaxDensity());
					}
					else
					{
						voxel.setMaterial(0);
						voxel.setDensity(Material8::getMinDensity());
					}

					if(y < 8)
					{
						voxel.setMaterial(239);
						voxel.setDensity(Material8::getMaxDensity());
					}

					pPolyVoxVolume->setVoxelAt(x,y,z,voxel);
				}
			}
		}

		return;
	}

	void TankWarsViewWidget::generateMengerSponge(Volume* volume)
	{
		const int mapWidth = 128;
		const int mapHeight = 128;
		const int mapDepth = 128;

		int spongeWidth = 1;
		while(spongeWidth < mapWidth / 3)
		{
			spongeWidth *= 3;
		}

		PolyVox::Volume<PolyVox::Material8>* pPolyVoxVolume = volume->m_pPolyVoxVolume;

		for(int z = 0; z < mapDepth; z++)
		{
			for(int y = 0; y < mapHeight; y++)
			{
				for(int x = 0; x < mapWidth; x++) 
				{
					bool solid = true;

					int i = x;
					int j = y;
					int k = z;

					int centred;

					for(int ct = 0; ct < 5; ct++)
					{
						centred = 0;
						if(i % 3 == 1)
							centred++;
						if(j % 3 == 1)
							centred++;
						if(k % 3 == 1)
							centred++;

						if(centred >= 2)
							solid = false;

						i/=3;
						j/=3;
						k/=3;
					}

					if(x >= spongeWidth)
						solid = false;
					if(y >= spongeWidth)
						solid = false;
					if(z >= spongeWidth)
						solid = false;

					

					Material8 voxel;
					if(solid)
					{
						voxel.setMaterial(60);
						voxel.setDensity(Material8::getMaxDensity());
					}
					else
					{
						voxel.setMaterial(0);
						voxel.setDensity(Material8::getMinDensity());
					}
					pPolyVoxVolume->setVoxelAt(x,y,z,voxel);
				}
			}
		}

		return;
	}
}
