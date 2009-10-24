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

#ifdef ENABLE_BULLET_PHYSICS
	#include "OgreBulletCollisions.h"
#endif //ENABLE_BULLET_PHYSICS

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

		void setPhysicsData(const PolyVox::IndexedSurfacePatch& isp);

		void addSurfacePatchRenderable(std::string materialName, PolyVox::IndexedSurfacePatch& isp, PolyVox::uint8_t uLodLevel);
		void removeAllSurfacePatchRenderablesForLod(PolyVox::uint8_t uLodLevel);

		void destroyPhysicsData(void);

		std::string makeUniqueName(const std::string& strBase);

		void setLodLevelToUse(PolyVox::uint8_t uLodLevel);

	private:
		Ogre::SceneNode* m_pOgreSceneNode;

		std::list<SurfacePatchRenderable*> m_listSingleMaterialSurfacePatchRenderablesLod0;
		std::list<SurfacePatchRenderable*> m_listSingleMaterialSurfacePatchRenderablesLod1;
		std::list<SurfacePatchRenderable*> m_listSingleMaterialSurfacePatchRenderablesLod2;

		std::list<SurfacePatchRenderable*> m_listMultiMaterialSurfacePatchRenderablesLod0;
		std::list<SurfacePatchRenderable*> m_listMultiMaterialSurfacePatchRenderablesLod1;
		std::list<SurfacePatchRenderable*> m_listMultiMaterialSurfacePatchRenderablesLod2;

#ifdef ENABLE_BULLET_PHYSICS
		void copyISPToTriangleMesh(const PolyVox::IndexedSurfacePatch& isp, btTriangleMesh* triMesh);
		void updateTriangleMeshWithNewISP(const PolyVox::IndexedSurfacePatch& isp, btTriangleMesh* triMesh);
#endif //ENABLE_BULLET_PHYSICS

	public:

		Map* m_pParentMap;

#ifdef ENABLE_BULLET_PHYSICS
		btTriangleMesh* mTriMesh;
		btBvhTriangleMeshShape* mShape;
		btRigidBody* mBody;
#endif //ENABLE_BULLET_PHYSICS

		static unsigned long m_iNameGen; //Used for unique names

		PolyVox::Vector3DInt16 m_v3dPos;
	};
}

#endif