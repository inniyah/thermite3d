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

#ifndef __PhysicalEntity_H__
#define __PhysicalEntity_H__

#ifdef ENABLE_BULLET_PHYSICS

#include "ThermiteForwardDeclarations.h"

#include "OgrePrerequisites.h"

#include "OgreBulletCollisionsPreRequisites.h"

#include "OgreBulletDynamicsPreRequisites.h"

namespace Thermite
{
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
}

#endif //ENABLE_BULLET_PHYSICS

#endif
