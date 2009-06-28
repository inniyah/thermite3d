#include "MapHandler.h"

#include <OgreEntity.h>
#include <OgreSceneManager.h>

#include "Application.h"
#include "Map.h"
#include "PhysicalEntity.h"
#include "VolumeManager.h"

#include "VolumeChangeTracker.h"

#include <QSettings>

using namespace PolyVox;

namespace Thermite
{
	MapHandler::MapHandler(Map* map)
	:DotSceneHandler(map->m_pOgreSceneManager)
	,mMap(map)
	{
		//mSceneManager = sceneManager;
	}

	bool MapHandler::startElement(const QString& namespaceURI,
									   const QString& localName,
									   const QString& qName,
									   const QXmlAttributes &attributes)
	{
		DotSceneHandler::startElement(namespaceURI, localName, qName, attributes);

		if(qName == "volume")
		{
			handleVolume(attributes);
		}

		if(qName == "voxel")
		{
			handleVoxel(attributes);
		}

		return true;
	}

	Ogre::Entity* MapHandler::handleEntity(const QXmlAttributes &attributes)
	{
		Ogre::Entity* entity = DotSceneHandler::handleEntity(attributes);

		if(qApp->settings()->value("Physics/SimulatePhysics", false).toBool())
		{
			float restitution = convertWithDefault(attributes.value("restitution"), 1.0f);
			float friction = convertWithDefault(attributes.value("friction"), 1.0f);
			float mass = convertWithDefault(attributes.value("mass"), 0.0f);

			QString collisionShape = convertWithDefault(attributes.value("collisionShape"), "box");
			PhysicalEntity::CollisionShapeType collisionShapeType;
			if(collisionShape.compare("box") == 0)
			{
				collisionShapeType = PhysicalEntity::CST_BOX;
			}
			else if(collisionShape.compare("sphere") == 0)
			{
				collisionShapeType = PhysicalEntity::CST_SPHERE;
			}
			else if(collisionShape.compare("convexHull") == 0)
			{
				collisionShapeType = PhysicalEntity::CST_CONVEX_HULL;
			}
			else if(collisionShape.compare("exact") == 0)
			{
				collisionShapeType = PhysicalEntity::CST_EXACT;
			}

			PhysicalEntity* physEnt = new PhysicalEntity(mMap, entity, restitution, friction, mass, collisionShapeType);
		}

		return entity;
	}

	void* MapHandler::handleVolume(const QXmlAttributes &attributes)
	{
		QString volumeSource = convertWithDefault(attributes.value("source"), "");

		mMap->volumeResource = VolumeManager::getSingletonPtr()->load(volumeSource.toStdString(), "General");
		if(mMap->volumeResource.isNull())
		{
			Ogre::LogManager::getSingleton().logMessage("Failed to load volume");
		}

		int regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();
		mMap->volumeChangeTracker = new VolumeChangeTracker(mMap->volumeResource->getVolume(), regionSideLength);

		mMap->volumeChangeTracker->setAllRegionsModified();

		return 0;
	}

	void* MapHandler::handleVoxel(const QXmlAttributes &attributes)
	{
		uint8_t voxelValue = convertWithDefault(attributes.value("value"), 0);
		QString voxelMaterial = convertWithDefault(attributes.value("material"), "");

		mMap->m_mapMaterialIds[voxelMaterial.toStdString()].insert(voxelValue);

		return 0;
	}
}