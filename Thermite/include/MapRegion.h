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

		void addSurfacePatchRenderable(std::string materialName, PolyVox::IndexedSurfacePatch& isp);
		void removeAllSurfacePatchRenderables();

		void destroyPhysicsData(void);

		std::string makeUniqueName(const std::string& strBase);

	private:
		Ogre::SceneNode* m_pOgreSceneNode;

		std::list<SurfacePatchRenderable*> m_listSingleMaterialSurfacePatchRenderables;

		std::list<SurfacePatchRenderable*> m_listMultiMaterialSurfacePatchRenderables;

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