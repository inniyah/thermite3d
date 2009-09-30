#include "MapHandler.h"

#include "Application.h"
#include "Map.h"
#include "PhysicalEntity.h"
#include "VolumeManager.h"

#include <OgreEntity.h>
#include <OgreSceneManager.h>

#include <QSettings>

using namespace PolyVox;

namespace Thermite
{
	MapHandler::MapHandler(Map* map)
	:DotSceneHandler(map->m_pOgreSceneManager)
	,mMap(map)
	{
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

#ifdef ENABLE_BULLET_PHYSICS
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

#endif //ENABLE_BULLET_PHYSICS

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

		return 0;
	}

	void* MapHandler::handleVoxel(const QXmlAttributes &attributes)
	{
		PolyVox::uint8_t voxelValue = convertWithDefault(attributes.value("value"), 0);
		QString voxelMaterial = convertWithDefault(attributes.value("material"), "");

		mMap->m_mapMaterialIds[voxelMaterial.toStdString()].insert(voxelValue);

		return 0;
	}
}