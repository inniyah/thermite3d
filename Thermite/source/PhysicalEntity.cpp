#pragma region License
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
#pragma endregion

#ifdef ENABLE_BULLET_PHYSICS

#include "Map.h"
#include "PhysicalEntity.h"
#include "ThermiteGameLogic.h"
#include "Utility.h"

#include "OgreEntity.h"
#include "OgreSceneNode.h"

#include "Shapes/OgreBulletCollisionsBoxShape.h"
#include "Shapes/OgreBulletCollisionsConvexHullShape.h"
#include "Shapes/OgreBulletCollisionsGImpactShape.h"
#include "Shapes/OgreBulletCollisionsSphereShape.h"
#include "Shapes/OgreBulletCollisionsTrimeshShape.h"
#include "Utils/OgreBulletCollisionsMeshToShapeConverter.h"
#include "OgreBulletDynamicsRigidBody.h"

using namespace OgreBulletCollisions;
using namespace OgreBulletDynamics;

namespace Thermite
{
	PhysicalEntity::PhysicalEntity(Map* pParentMap, Ogre::Entity* entity, float restitution, float friction, float mass, CollisionShapeType collisionShapeType)
		:m_pEntity(entity)
		,m_pSceneNode(0)
		,m_pCollisionShape(0)
		,m_pRigidBody(0)
		,m_pParentMap(pParentMap)
	{
		m_pSceneNode = entity->getParentSceneNode();

		//This code is not fully understood. It seems the 'transform' parameter which is passed to StaticMeshToShapeConverter
		//should be just the scale - passing the nodes '_getFullTransform()' seems to cause problems. Therefore the position
		//and orientation are extracted and passed seperatly when the shape is set on the rigid body later on.
		//Also, note that the position and orientation are stored in variables which are passed to 'setShape()' later.
		//Embedding m_pSceneNode->getPosition() directly into the setShape() call seems to cause problems because the position
		//is passed by reference and then set back on the scene node within setShape(). Or something :-S
		Ogre::Matrix4 scale = Ogre::Matrix4::IDENTITY;	
		scale.setScale(m_pSceneNode->getScale());
		Ogre::Vector3 position = m_pSceneNode->getPosition();
		Ogre::Quaternion orientation = m_pSceneNode->getOrientation();	

		//Create our desired collision shape.
		StaticMeshToShapeConverter *converter = new StaticMeshToShapeConverter(entity, scale);
		switch(collisionShapeType)
		{
		case CST_BOX:
			{
				m_pCollisionShape = converter->createBox();
				break;
			}
		case CST_SPHERE:
			{
				m_pCollisionShape = converter->createSphere();
				break;
			}
		case CST_CONVEX_HULL:
			{
				m_pCollisionShape = converter->createConvex();
				break;
			}
		case CST_EXACT:
			{			
				m_pCollisionShape = converter->createConcave();
				break;
			}
		}
		
		//Create the rigid body and set the colision shape for it.
		std::string name = Thermite::generateUID("PO_RB");
		m_pRigidBody = new RigidBody(name, m_pParentMap->m_pOgreBulletWorld);
		m_pRigidBody->setShape (m_pSceneNode,  m_pCollisionShape, restitution, friction, mass, position, orientation);
	}

	PhysicalEntity::~PhysicalEntity()
	{
		//Ogre::LogManager::getSingleton().logMessage("Removing Physical Object");
		//m_pParentMap->m_pOgreBulletWorld->removeObject(m_pRigidBody);
		delete m_pRigidBody;
		m_pRigidBody = 0;

		m_pSceneNode->detachObject(m_pEntity);
		m_pParentMap->m_pOgreSceneManager->destroyEntity(m_pEntity);
		m_pParentMap->m_pOgreSceneManager->getRootSceneNode()->removeAndDestroyChild(m_pSceneNode->getName());

		m_pSceneNode = 0;
		m_pEntity = 0;



		/*delete m_pCollisionShape;
		m_pCollisionShape = 0;*/
	}
}

#endif //ENABLE_BULLET_PHYSICS