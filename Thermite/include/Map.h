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
#include "MultiThreadedSurfaceExtractor.h"

#include "PolyVoxForwardDeclarations.h"
#include "VolumeChangeTracker.h"

#include "OgreAxisAlignedBox.h"
#include "OgreVector3.h"

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
		Map(Ogre::SceneManager* sceneManager);
		~Map(void);

		void initialisePhysics(void);

		bool loadScene(const Ogre::String& filename);

		void updatePolyVoxGeometry();	
		void updateLOD(void);

	public:
		Ogre::SceneManager* m_pOgreSceneManager;

		PolyVox::VolumeChangeTracker* volumeChangeTracker;

		VolumeResourcePtr volumeResource;

#ifdef ENABLE_BULLET_PHYSICS
		OgreBulletDynamics::DynamicsWorld *m_pOgreBulletWorld;
#endif //ENABLE_BULLET_PHYSICS

		PolyVox::Volume<MapRegion*>* m_volMapRegions;

		MultiThreadedSurfaceExtractor* m_pMTSE;

		ThermiteGameLogic* m_pThermiteGameLogic; //Nasty hack to allow progress monitoring

		std::map< std::string, std::set<PolyVox::uint8_t> > m_mapMaterialIds;

	private:
		Ogre::Camera* m_pCamera;

		PolyVox::Volume<PolyVox::uint32_t>* m_volRegionTimeStamps;

		int m_iNoProcessed;
		int m_iNoSubmitted;
	};
}


#endif