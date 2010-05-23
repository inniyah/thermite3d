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

#include "MapRegion.h"

#include "Application.h"
#include "Map.h"
#include "SurfacePatchRenderable.h"
#include "ThermiteGameLogic.h"
#include "Vector.h"

#ifdef ENABLE_BULLET_PHYSICS
	#include "OgreBulletDynamicsRigidBody.h"
#endif //ENABLE_BULLET_PHYSICS

#include "OgreVector3.h"

#include <QSettings>

//using namespace Ogre;
#ifdef ENABLE_BULLET_PHYSICS
using namespace OgreBulletCollisions;
using namespace OgreBulletDynamics;
#endif //ENABLE_BULLET_PHYSICS
using namespace PolyVox;

namespace Thermite
{
	unsigned long MapRegion::m_iNameGen = 0;

	MapRegion::MapRegion(Map* pParentMap, PolyVox::Vector3DInt16 v3dPos)
	:m_pOgreSceneNode(0)
	//,m_pSurfacePatchRenderable(0)
	,m_pParentMap(pParentMap)
#ifdef ENABLE_BULLET_PHYSICS
	,mBody(0)
	,mTriMesh(0)
	,mShape(0)
#endif //ENABLE_BULLET_PHYSICS
	{
		m_v3dPos = v3dPos;

		const std::string& strNodeName = makeUniqueName("SN");
		m_pOgreSceneNode = m_pParentMap->m_pOgreSceneManager->getRootSceneNode()->createChildSceneNode(strNodeName);
		m_pOgreSceneNode->setPosition(Ogre::Vector3(m_v3dPos.getX(),m_v3dPos.getY(),m_v3dPos.getZ()));
	}

	MapRegion::~MapRegion()
	{
		/*if(m_pSurfacePatchRenderable != 0)
		{
			if(m_pOgreSceneNode != 0)
			{
				m_pOgreSceneNode->detachObject(m_pSurfacePatchRenderable);
			}
			m_pParentMap->m_pOgreSceneManager->destroyMovableObject(m_pSurfacePatchRenderable);
			m_pSurfacePatchRenderable = 0;
		}*/

		if(m_pOgreSceneNode != 0)
		{
			m_pParentMap->m_pOgreSceneManager->getRootSceneNode()->removeAndDestroyChild(m_pOgreSceneNode->getName());
			m_pOgreSceneNode = 0;
		}
	}

	void MapRegion::addSurfacePatchRenderable(std::string materialName, SurfaceMesh& mesh)
	{
		//Single Material
		SurfacePatchRenderable* pSingleMaterialSurfacePatchRenderable;

		std::string strSprName = makeUniqueName("SPR");
		pSingleMaterialSurfacePatchRenderable = dynamic_cast<SurfacePatchRenderable*>(m_pParentMap->m_pOgreSceneManager->createMovableObject(strSprName, SurfacePatchRenderableFactory::FACTORY_TYPE_NAME));
		pSingleMaterialSurfacePatchRenderable->setMaterial(materialName);
		pSingleMaterialSurfacePatchRenderable->setCastShadows(qApp->settings()->value("Shadows/EnableShadows", false).toBool());
		m_pOgreSceneNode->attachObject(pSingleMaterialSurfacePatchRenderable);
		pSingleMaterialSurfacePatchRenderable->m_v3dPos = m_pOgreSceneNode->getPosition();

		pSingleMaterialSurfacePatchRenderable->buildRenderOperationFrom(mesh, true);

		int regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();
		Ogre::AxisAlignedBox aabb(Ogre::Vector3(0.0f,0.0f,0.0f), Ogre::Vector3(regionSideLength, regionSideLength, regionSideLength));
		pSingleMaterialSurfacePatchRenderable->setBoundingBox(aabb);

		
		m_listSingleMaterialSurfacePatchRenderables.push_back(pSingleMaterialSurfacePatchRenderable);

		//Multi material
		SurfacePatchRenderable* pMultiMaterialSurfacePatchRenderable;

		//Create additive material
		Ogre::String strAdditiveMaterialName = materialName + "_WITH_ADDITIVE_BLENDING";
		Ogre::MaterialPtr additiveMaterial = Ogre::MaterialManager::getSingleton().getByName(strAdditiveMaterialName);
		if(additiveMaterial.isNull() == true)
		{
			Ogre::MaterialPtr originalMaterial = Ogre::MaterialManager::getSingleton().getByName(materialName);
			additiveMaterial = originalMaterial->clone(strAdditiveMaterialName);
			additiveMaterial->setSceneBlending(Ogre::SBT_ADD);
		}

		strSprName = makeUniqueName("SPR");
		pMultiMaterialSurfacePatchRenderable = dynamic_cast<SurfacePatchRenderable*>(m_pParentMap->m_pOgreSceneManager->createMovableObject(strSprName, SurfacePatchRenderableFactory::FACTORY_TYPE_NAME));
		pMultiMaterialSurfacePatchRenderable->setMaterial(strAdditiveMaterialName);
		pMultiMaterialSurfacePatchRenderable->setCastShadows(qApp->settings()->value("Shadows/EnableShadows", false).toBool());
		m_pOgreSceneNode->attachObject(pMultiMaterialSurfacePatchRenderable);
		pMultiMaterialSurfacePatchRenderable->m_v3dPos = m_pOgreSceneNode->getPosition();

		pMultiMaterialSurfacePatchRenderable->buildRenderOperationFrom(mesh, false);

		//int regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();
		//Ogre::AxisAlignedBox aabb(Ogre::Vector3(0.0f,0.0f,0.0f), Ogre::Vector3(regionSideLength, regionSideLength, regionSideLength));
		pMultiMaterialSurfacePatchRenderable->setBoundingBox(aabb);

		m_listMultiMaterialSurfacePatchRenderables.push_back(pMultiMaterialSurfacePatchRenderable);
	}

	void MapRegion::removeAllSurfacePatchRenderables(void)
	{
		for(std::list<SurfacePatchRenderable*>::iterator iter = m_listSingleMaterialSurfacePatchRenderables.begin(); iter != m_listSingleMaterialSurfacePatchRenderables.end(); iter++)
		{
			if(m_pOgreSceneNode != 0)
			{
				m_pOgreSceneNode->detachObject(*iter);
			}
			m_pParentMap->m_pOgreSceneManager->destroyMovableObject(*iter);
		}

		m_listSingleMaterialSurfacePatchRenderables.clear();

		for(std::list<SurfacePatchRenderable*>::iterator iter = m_listMultiMaterialSurfacePatchRenderables.begin(); iter != m_listMultiMaterialSurfacePatchRenderables.end(); iter++)
		{
			if(m_pOgreSceneNode != 0)
			{
				m_pOgreSceneNode->detachObject(*iter);
			}
			m_pParentMap->m_pOgreSceneManager->destroyMovableObject(*iter);
		}

		m_listMultiMaterialSurfacePatchRenderables.clear();
	}

	void MapRegion::setPhysicsData(const SurfaceMesh& mesh)
	{
#ifdef ENABLE_BULLET_PHYSICS
		const std::string& strName = makeUniqueName("RB");

		destroyPhysicsData();

		if(mesh.isEmpty())
		{
			return;
		}

		const float      gDynamicBodyRestitution = 0.6f;
		const float      gDynamicBodyFriction    = 0.6f;
		const float      gDynamicBodyMass        = 1.0f;

		/*if(mTriMesh == 0)
		{*/
			mTriMesh = new btTriangleMesh();
			copyMeshToTriangleMesh(mesh, mTriMesh);   
		/*}
		else
		{
			updateTriangleMeshWithNewMesh(mesh, mTriMesh);
		}*/

		const btVector3 minAabb(0,0,0);
		int regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();
		const btVector3 maxAabb(regionSideLength,regionSideLength,regionSideLength);
		const bool useQuantizedAABB = true;
		//if(mShape == 0)
		{
			mShape = new btBvhTriangleMeshShape(mTriMesh, useQuantizedAABB, minAabb, maxAabb);
		}
		/*else
		{
			mShape->m_meshInterface = mTriMesh;
		}*/

		/*if(mBody == 0)
		{*/
			mBody = new btRigidBody(0.0, 0, mShape);

			mBody->setRestitution(gDynamicBodyRestitution);
			mBody->setFriction(gDynamicBodyFriction);

			mBody->getWorldTransform().setOrigin(btVector3( m_v3dPos.getX(),  m_v3dPos.getY(), m_v3dPos.getZ()));
			mBody->getWorldTransform().setRotation(btQuaternion(0, 0, 0, 1));

			m_pParentMap->m_pOgreBulletWorld->getBulletDynamicsWorld()->addRigidBody(mBody);
		/*}
		else
		{
			mBody->setCollisionShape(mShape);
		}*/
#endif //ENABLE_BULLET_PHYSICS
	}

	void MapRegion::destroyPhysicsData(void)
	{	
#ifdef ENABLE_BULLET_PHYSICS
		if(mBody != 0)
		{
			m_pParentMap->m_pOgreBulletWorld->getBulletDynamicsWorld()->removeRigidBody(mBody);
			delete mBody;
			mBody = 0;
		}
		/*if(m_pRigidBody != 0)
		{
			//m_pParentMap->m_pOgreBulletWorld->removeObject(m_pRigidBody);					
			delete m_pRigidBody;
			m_pRigidBody = 0;
		}*/
		/*if(m_pTriangleMeshCollisionShape != 0)
		{
			delete m_pTriangleMeshCollisionShape;
			m_pTriangleMeshCollisionShape = 0;
		}*/
#endif //ENABLE_BULLET_PHYSICS
	}

	std::string MapRegion::makeUniqueName(const std::string& strBase)
	{
		std::stringstream ssName;
		ssName << strBase << " " << ++m_iNameGen;
		return ssName.str();
	}

#ifdef ENABLE_BULLET_PHYSICS
	void MapRegion::copyMeshToTriangleMesh(const PolyVox::SurfaceMesh& mesh, btTriangleMesh* triMesh)
	{
		btVector3    vecBulletVertices[3];

		std::vector<uint32_t>::const_iterator indicesIter = mesh.getIndices().begin();
		while(indicesIter != mesh.getIndices().end())
		{
			for (unsigned int v = 0; v < 3; ++v)
			{
				uint32_t uIndex = *indicesIter;
				const SurfaceVertex& vertex = mesh.getVertices()[uIndex];
				const Vector3DFloat& vertexPos = vertex.getPosition();

				vecBulletVertices[v][0] = vertex.getPosition().getX();
				vecBulletVertices[v][1] = vertex.getPosition().getY();
				vecBulletVertices[v][2] = vertex.getPosition().getZ();

				++indicesIter;
			}

			mTriMesh->addTriangle(vecBulletVertices[0], vecBulletVertices[1], vecBulletVertices[2]);
		}
	}

	void MapRegion::updateTriangleMeshWithNewMesh(const PolyVox::SurfaceMesh& mesh, btTriangleMesh* triMesh)
	{
		mTriMesh->preallocateVertices(mesh.getVertices().size());
		mTriMesh->preallocateIndices(mesh.getIndices().size());
		unsigned char *vertexbase;
		int numverts;
		PHY_ScalarType type;
		int stride;
		unsigned char *indexbase;
		int indexstride;
		int numfaces;
		PHY_ScalarType indicestype;
		int subpart=0;

		mTriMesh->getLockedVertexIndexBase(&vertexbase, numverts,type,stride,&indexbase,indexstride,numfaces,indicestype,subpart);

		Ogre::LogManager::getSingleton().logMessage("Vertex Stride is " + Ogre::StringConverter::toString(stride));
		Ogre::LogManager::getSingleton().logMessage("Index Stride is " + Ogre::StringConverter::toString(indexstride));

		float* vertexBaseAsFloat = reinterpret_cast<float*>(vertexbase);
		std::vector<SurfaceVertex>::const_iterator verticesIter = mesh.getVertices().begin();
		//LogManager::getSingleton().logMessage("copying vertices");
		int ct = 0;
		while(verticesIter != mesh.getVertices().end())
		{
			//LogManager::getSingleton().logMessage("copying vertex " + StringConverter::toString(ct));
			*vertexBaseAsFloat = (float)(verticesIter->getPosition().getX());
			++vertexBaseAsFloat;
			*vertexBaseAsFloat = (float)(verticesIter->getPosition().getY());
			++vertexBaseAsFloat;
			*vertexBaseAsFloat = (float)(verticesIter->getPosition().getZ());
			++vertexBaseAsFloat;
			++vertexBaseAsFloat;

			++verticesIter;
			ct++;
		}

		//LogManager::getSingleton().logMessage("copying indices");
		int* indexBaseAsInt = reinterpret_cast<int*>(indexbase);
		std::vector<uint32_t>::const_iterator indicesIter = mesh.getIndices().begin();
		while(indicesIter != mesh.getIndices().end())
		{
			*indexBaseAsInt = (int)(*indicesIter);
			++indexBaseAsInt;
			++indicesIter;
		}

		//LogManager::getSingleton().logMessage("unlocking");

		mTriMesh->unLockVertexBase(subpart);
	}
#endif //ENABLE_BULLET_PHYSICS
}