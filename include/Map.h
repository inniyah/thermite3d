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

#ifndef __World_H__
#define __World_H__

#include "ThermiteForwardDeclarations.h"
#include "VolumeResource.h"

#include "PolyVoxCore/PolyVoxForwardDeclarations.h"
#include "PolyVoxUtil/VolumeChangeTracker.h"

#include "OgreBulletDynamicsWorld.h"
#include "OgreBulletDynamicsRigidBody.h"

#include <OgrePrerequisites.h>

#include <map>

class Map
{
public:
	Map(Ogre::Vector3 vecGravity, Ogre::AxisAlignedBox boxPhysicsBounds, Ogre::Real rVoxelSize, Ogre::SceneManager* sceneManager);
	~Map(void);

	void initialisePhysics(void);

	bool Map::loadScene(const Ogre::String& filename);

	void updatePolyVoxGeometry();	

	void createAxis(unsigned int uSideLength);

public:
	Ogre::SceneManager* m_pOgreSceneManager;

	PolyVox::VolumeChangeTracker* volumeChangeTracker;

	VolumeResourcePtr volumeResource;

	OgreBulletDynamics::DynamicsWorld *m_pOgreBulletWorld;

	Ogre::SceneNode* makeSureSceneNodeExists(bool bShouldExist, const Ogre::String strSceneNodeName);

	std::map<PolyVox::Vector3DInt32, WorldRegion*> m_mapWorldRegions;

private:
	Ogre::Vector3 m_vecGravity;
	Ogre::AxisAlignedBox m_boxPhysicsBounds;
	Ogre::Real m_rVoxelSize;

	Ogre::Timer* timer;

	Ogre::SceneNode* m_axisNode;

	PolyVox::int32_t m_iRegionTimeStamps[THERMITE_VOLUME_SIDE_LENGTH_IN_REGIONS][THERMITE_VOLUME_SIDE_LENGTH_IN_REGIONS][THERMITE_VOLUME_SIDE_LENGTH_IN_REGIONS];
};


#endif