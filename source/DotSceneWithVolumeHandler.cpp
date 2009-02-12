#include "DotSceneWithVolumeHandler.h"

#include <OgreEntity.h>
#include <OgreSceneManager.h>

#include "PhysicalEntity.h"
#include "World.h"

DotSceneWithVolumeHandler::DotSceneWithVolumeHandler(World* world)
:DotSceneHandler(world->m_pOgreSceneManager)
,mWorld(world)
{
	//mSceneManager = sceneManager;
}

Ogre::Entity* DotSceneWithVolumeHandler::handleEntity(const QXmlAttributes &attributes)
{
	Ogre::Entity* entity = DotSceneHandler::handleEntity(attributes);

	float restitution = convertWithDefault(attributes.value("restitution"), 1.0f);
	float friction = convertWithDefault(attributes.value("friction"), 1.0f);
	float mass = convertWithDefault(attributes.value("mass"), 1.0f);

	QString collisionShape = convertWithDefault(attributes.value("collisionShape"), "box");
	PhysicalEntity::CollisionShapeType collisionShapeType;
	if(collisionShape.compare("box"))
	{
		collisionShapeType = PhysicalEntity::CST_BOX;
	}
	else if(collisionShape.compare("sphere"))
	{
		collisionShapeType = PhysicalEntity::CST_SPHERE;
	}
	else if(collisionShape.compare("convexHull"))
	{
		collisionShapeType = PhysicalEntity::CST_CONVEX_HULL;
	}
	else if(collisionShape.compare("exact"))
	{
		collisionShapeType = PhysicalEntity::CST_EXACT;
	}

	PhysicalEntity* physEnt = new PhysicalEntity(mWorld, entity, restitution, friction, mass, collisionShapeType);

	return entity;
}