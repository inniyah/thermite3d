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

#ifndef __THERMITE_MAP_H__
#define __THERMITE_MAP_H__

#include "ThermiteForwardDeclarations.h"
#include "VolumeResource.h"

#include "PolyVoxForwardDeclarations.h"

#ifdef ENABLE_BULLET_PHYSICS
	#include "OgreBulletDynamicsWorld.h"
	#include "OgreBulletDynamicsRigidBody.h"
#endif //ENABLE_BULLET_PHYSICS

#include <OgrePrerequisites.h>

#include <map>

namespace Thermite
{
	class Map
	{
	public:
#ifdef ENABLE_BULLET_PHYSICS
		//Map(Ogre::SceneManager* sceneManager, OgreBulletDynamics::DynamicsWorld *pOgreBulletWorld);
#else
		//Map(Ogre::SceneManager* sceneManager);
#endif //ENABLE_BULLET_PHYSICS
		Map();
		~Map(void);

	public:
		Ogre::SceneManager* m_pOgreSceneManager;
#ifdef ENABLE_BULLET_PHYSICS
		OgreBulletDynamics::DynamicsWorld *m_pOgreBulletWorld;
#endif //ENABLE_BULLET_PHYSICS

		VolumeResourcePtr volumeResource;

		std::map< std::string, std::set<uint8_t> > m_mapMaterialIds;	
	};
}


#endif