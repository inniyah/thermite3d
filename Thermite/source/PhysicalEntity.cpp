#pragma region License
/******************************************************************************
This file is part of the Thermite 3D game engine
Copyright (C) 2006  David Williams

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
******************************************************************************/
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