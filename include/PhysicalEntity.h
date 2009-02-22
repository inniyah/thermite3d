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

#ifndef __PhysicalEntity_H__
#define __PhysicalEntity_H__

#include "ThermiteForwardDeclarations.h"

#include "OgrePrerequisites.h"

#include "OgreBulletCollisionsPreRequisites.h"

#include "OgreBulletDynamicsPreRequisites.h"

class PhysicalEntity
{
public:
	enum CollisionShapeType
	{
		CST_BOX,
		CST_SPHERE,
		CST_CONVEX_HULL,
		CST_EXACT
	};

	PhysicalEntity(Map* pParentMap , Ogre::Entity* entity, float restitution, float friction, float mass, CollisionShapeType collisionShapeType = CST_BOX);
	~PhysicalEntity();

	Ogre::Entity* m_pEntity;
	Ogre::SceneNode* m_pSceneNode;
	OgreBulletCollisions::CollisionShape* m_pCollisionShape;
	OgreBulletDynamics::RigidBody* m_pRigidBody;
	Map* m_pParentMap;
};

#endif
