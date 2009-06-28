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

#ifndef __MapRegion_H__
#define __MapRegion_H__

#include "PolyVoxForwardDeclarations.h"
#include "IndexedSurfacePatch.h"

#include "ThermiteForwardDeclarations.h"

#include <OgrePrerequisites.h>

#include "OgreBulletCollisions.h"

namespace OgreBulletDynamics
{
	class RigidBody;
}

namespace Thermite
{
	class MapRegion
	{
	public:
		MapRegion(Map* pParentWorld, PolyVox::Vector3DInt16 v3dPos);
		~MapRegion();

		void setPhysicsData(Ogre::Vector3 pos, const PolyVox::IndexedSurfacePatch& isp);

		void update(PolyVox::IndexedSurfacePatch* ispNew);

		void addSurfacePatchRenderable(std::string materialName, PolyVox::IndexedSurfacePatch& isp, PolyVox::uint8_t uLodLevel);
		void removeAllSurfacePatchRenderablesForLod(PolyVox::uint8_t uLodLevel);

		void destroyPhysicsData(void);

		std::string makeUniqueName(const std::string& strBase);

		void setLodLevelToUse(PolyVox::uint8_t uLodLevel);

	private:
		Ogre::SceneNode* m_pOgreSceneNode;

		std::list<SurfacePatchRenderable*> m_listSurfacePatchRenderablesLod0;
		std::list<SurfacePatchRenderable*> m_listSurfacePatchRenderablesLod1;
		std::list<SurfacePatchRenderable*> m_listSurfacePatchRenderablesLod2;

		void copyISPToTriangleMesh(const PolyVox::IndexedSurfacePatch& isp, btTriangleMesh* triMesh);
		void updateTriangleMeshWithNewISP(const PolyVox::IndexedSurfacePatch& isp, btTriangleMesh* triMesh);

	public:

		Map* m_pParentMap;

		btTriangleMesh* mTriMesh;
		btBvhTriangleMeshShape* mShape;
		btRigidBody* mBody;

		static unsigned long m_iNameGen; //Used for unique names

		PolyVox::Vector3DInt16 m_v3dPos;
	};
}

#endif