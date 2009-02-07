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

#include "PhysicalObject.h"

#include "World.h"

#include "OgreEntity.h"
#include "OgreSceneNode.h"

#include "Shapes/OgreBulletCollisionsBoxShape.h"
#include "Shapes/OgreBulletCollisionsSphereShape.h"
#include "OgreBulletDynamicsRigidBody.h"

using namespace Ogre;
using namespace OgreBulletCollisions;
using namespace OgreBulletDynamics;

unsigned long PhysicalObject::m_iNameGen = 0;
unsigned long PhysicalObject::m_iNextObject = 0;

PhysicalObject::PhysicalObject(World* pParentWorld, std::string strMeshName, Vector3 vecInitialPos)
:m_pEntity(0)
,m_pSceneNode(0)
,m_pCollisionShape(0)
,m_pRigidBody(0)
,m_pParentWorld(pParentWorld)
{
	const float      gDynamicBodyRestitution = 0.6f;
	const float      gDynamicBodyFriction    = 0.6f;
	const float      gDynamicBodyMass        = 1.0f;

	Vector3 size(5.0,5.0,5.0);
	//Vector3 pos(xPos,yPos,128.0);

	if(m_iNextObject % 2 == 0)
	{
		//Add a cube
		m_pEntity = m_pParentWorld->m_pOgreSceneManager->createEntity(makeUniqueName("PO_ENT"),"bulletbox.mesh");
		m_pCollisionShape = new BoxCollisionShape(size);
	}
	else
	{
		//Add a sphere
		m_pEntity = m_pParentWorld->m_pOgreSceneManager->createEntity(makeUniqueName("PO_ENT"),"ellipsoid.mesh");
		m_pEntity->setMaterialName("Bullet/Ball");
		m_pCollisionShape = new SphereCollisionShape(5.0);
	}
	m_iNextObject++;

	m_pRigidBody = new RigidBody(makeUniqueName("PO_RB"), m_pParentWorld->m_pOgreBulletWorld);
	m_pSceneNode = m_pParentWorld->m_pOgreSceneManager->getRootSceneNode()->createChildSceneNode ();
	m_pSceneNode->attachObject (m_pEntity);
	m_pSceneNode->scale(size);
	m_pRigidBody->setShape (m_pSceneNode,  m_pCollisionShape, gDynamicBodyRestitution, gDynamicBodyFriction, gDynamicBodyMass, vecInitialPos, Quaternion(0,0.5,0,1));
	m_pRigidBody->setLinearVelocity(0.0,0.0,0.0);
}

PhysicalObject::PhysicalObject(World* pParentWorld , Ogre::Entity* entity)
	:m_pEntity(entity)
	,m_pSceneNode(0)
	,m_pCollisionShape(0)
	,m_pRigidBody(0)
	,m_pParentWorld(pParentWorld)
{
	const float      gDynamicBodyRestitution = 0.6f;
	const float      gDynamicBodyFriction    = 0.6f;
	const float      gDynamicBodyMass        = 1.0f;

	Vector3 size(3.0,3.0,3.0);

	m_pSceneNode = entity->getParentSceneNode();

	m_pCollisionShape = new BoxCollisionShape(size);

	m_pRigidBody = new RigidBody(makeUniqueName("PO_RB"), m_pParentWorld->m_pOgreBulletWorld);
	//m_pSceneNode = m_pParentWorld->m_pOgreSceneManager->getRootSceneNode()->createChildSceneNode ();
	//m_pSceneNode->attachObject (m_pEntity);
	//m_pSceneNode->scale(size);
	Vector3 position = m_pSceneNode->getPosition();
	//m_pSceneNode->setPosition(0.0,0.0,0.0);
	m_pRigidBody->setShape (m_pSceneNode,  m_pCollisionShape, gDynamicBodyRestitution, gDynamicBodyFriction, gDynamicBodyMass, position, Quaternion(0,0.5,0,1));
	m_pRigidBody->setLinearVelocity(0.0,0.0,0.0);
}

PhysicalObject::~PhysicalObject()
{
	//Ogre::LogManager::getSingleton().logMessage("Removing Physical Object");
	//m_pParentWorld->m_pOgreBulletWorld->removeObject(m_pRigidBody);
	delete m_pRigidBody;
	m_pRigidBody = 0;

	m_pSceneNode->detachObject(m_pEntity);
	m_pParentWorld->m_pOgreSceneManager->destroyEntity(m_pEntity);
	m_pParentWorld->m_pOgreSceneManager->getRootSceneNode()->removeAndDestroyChild(m_pSceneNode->getName());

	m_pSceneNode = 0;
	m_pEntity = 0;



	/*delete m_pCollisionShape;
	m_pCollisionShape = 0;*/
}

std::string PhysicalObject::makeUniqueName(const std::string& strBase)
{
	std::stringstream ssName;
	ssName << strBase << " " << ++m_iNameGen;
	return ssName.str();
}