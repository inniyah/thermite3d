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

#ifndef __WorldRegion_H__
#define __WorldRegion_H__

#include "PolyVoxForwardDeclarations.h"
#include "IndexedSurfacePatch.h"

#include "ThermiteForwardDeclarations.h"

#include <OgrePrerequisites.h>

#include "OgreBulletCollisions.h"

namespace OgreBulletDynamics
{
	class RigidBody;
}

class WorldRegion
{
public:
	WorldRegion(Map* pParentWorld, PolyVox::Vector3DInt32 v3dPos);
	~WorldRegion();

	void setPhysicsData(Ogre::Vector3 pos, PolyVox::IndexedSurfacePatch& isp);

	void destroyPhysicsData(void);

	std::string makeUniqueName(const std::string& strBase);

private:
	Ogre::SceneNode* m_pOgreSceneNode;

	void copyISPToTriangleMesh(const PolyVox::IndexedSurfacePatch& isp, btTriangleMesh* triMesh);
	void updateTriangleMeshWithNewISP(const PolyVox::IndexedSurfacePatch& isp, btTriangleMesh* triMesh);

private:
	SurfacePatchRenderable* m_pSurfacePatchRenderable;

	Map* m_pParentMap;

	btTriangleMesh* mTriMesh;
	btBvhTriangleMeshShape* mShape;
	btRigidBody* mBody;

	static unsigned long m_iNameGen; //Used for unique names

	PolyVox::Vector3DInt32 m_v3dPos;
};

#endif