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
#include "Shapes/OgreBulletCollisionsConvexHullShape.h"
#include "Shapes/OgreBulletCollisionsGImpactShape.h"
#include "Shapes/OgreBulletCollisionsSphereShape.h"
#include "Shapes/OgreBulletCollisionsTrimeshShape.h"
#include "Utils/OgreBulletCollisionsMeshToShapeConverter.h"
#include "OgreBulletDynamicsRigidBody.h"

using namespace Ogre;
using namespace OgreBulletCollisions;
using namespace OgreBulletDynamics;

unsigned long PhysicalObject::m_iNameGen = 0;
unsigned long PhysicalObject::m_iNextObject = 0;

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

	//Vector3 size(3.0,3.0,3.0);

	m_pSceneNode = entity->getParentSceneNode();

	Vector3 position = m_pSceneNode->getPosition();

	Matrix4 scale = Matrix4::IDENTITY;
	
	scale.setScale(m_pSceneNode->getScale());
	//scale.setTrans(Vector3(0.0,0.0,0.0));

	//Matrix4 scale = m_pSceneNode->getScale();

	//m_pCollisionShape = new BoxCollisionShape(size);
	StaticMeshToShapeConverter *trimeshConverter = new StaticMeshToShapeConverter(entity, scale);
    //m_pCollisionShape = trimeshConverter->createConvex();
	m_pCollisionShape = new GImpactConcaveShape(trimeshConverter->mVertexBuffer, trimeshConverter->mVertexCount, trimeshConverter->mIndexBuffer, trimeshConverter->mIndexCount);

	m_pRigidBody = new RigidBody(makeUniqueName("PO_RB"), m_pParentWorld->m_pOgreBulletWorld);
	//m_pSceneNode = m_pParentWorld->m_pOgreSceneManager->getRootSceneNode()->createChildSceneNode ();
	//m_pSceneNode->attachObject (m_pEntity);
	//m_pSceneNode->scale(size);
	
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