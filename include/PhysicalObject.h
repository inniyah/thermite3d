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

#ifndef __PhysicalObject_H__
#define __PhysicalObject_H__

#include "ThermiteForwardDeclarations.h"

#include "OgrePrerequisites.h"

#include "OgreBulletCollisionsPreRequisites.h"

#include "OgreBulletDynamicsPreRequisites.h"

class PhysicalObject
{
public:
	PhysicalObject(World* pParentWorld ,std::string strMeshName, Ogre::Vector3 vecInitialPos);
	~PhysicalObject();

	std::string makeUniqueName(const std::string& strBase);

	Ogre::Entity* m_pEntity;
	Ogre::SceneNode* m_pSceneNode;
	OgreBulletCollisions::CollisionShape* m_pCollisionShape;
	OgreBulletDynamics::RigidBody* m_pRigidBody;
	World* m_pParentWorld;
	static unsigned long m_iNameGen; //Used for unique names
	static unsigned long m_iNextObject; //Used to cycle through objects whic are dropped.
};

#endif
