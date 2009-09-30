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

		std::map< std::string, std::set<PolyVox::uint8_t> > m_mapMaterialIds;	
	};
}


#endif