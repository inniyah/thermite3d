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

#include "MapRegion.h"

#include "Application.h"
#include "Map.h"
#include "SurfacePatchRenderable.h"
#include "Vector.h"

#include "OgreBulletDynamicsRigidBody.h"

#include "OgreVector3.h"

#include <QSettings>

using namespace Ogre;
using namespace OgreBulletCollisions;
using namespace OgreBulletDynamics;
using namespace PolyVox;

namespace Thermite
{
	unsigned long MapRegion::m_iNameGen = 0;

	MapRegion::MapRegion(Map* pParentMap, PolyVox::Vector3DInt16 v3dPos)
	:m_pOgreSceneNode(0)
	//,m_pSurfacePatchRenderable(0)
	,m_pParentMap(pParentMap)
	,mBody(0)
	,mTriMesh(0)
	,mShape(0)
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

	void MapRegion::addSurfacePatchRenderable(std::string materialName, IndexedSurfacePatch& isp, PolyVox::uint8_t uLodLevel)
	{
		SurfacePatchRenderable* pSurfacePatchRenderable;

		const std::string& strSprName = makeUniqueName("SPR");
		pSurfacePatchRenderable = dynamic_cast<SurfacePatchRenderable*>(m_pParentMap->m_pOgreSceneManager->createMovableObject(strSprName, SurfacePatchRenderableFactory::FACTORY_TYPE_NAME));
		pSurfacePatchRenderable->setMaterial(materialName);
		pSurfacePatchRenderable->setCastShadows(qApp->settings()->value("Shadows/EnableShadows", false).toBool());
		m_pOgreSceneNode->attachObject(pSurfacePatchRenderable);
		pSurfacePatchRenderable->m_v3dPos = m_pOgreSceneNode->getPosition();

		pSurfacePatchRenderable->buildRenderOperationFrom(isp);

		int regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();
		AxisAlignedBox aabb(Vector3(0.0f,0.0f,0.0f), Vector3(regionSideLength, regionSideLength, regionSideLength));
		pSurfacePatchRenderable->setBoundingBox(aabb);

		switch(uLodLevel)
		{
		case 0:
			m_listSurfacePatchRenderablesLod0.push_back(pSurfacePatchRenderable);
			break;
		case 1:
			m_listSurfacePatchRenderablesLod1.push_back(pSurfacePatchRenderable);
			break;
		case 2:
			m_listSurfacePatchRenderablesLod2.push_back(pSurfacePatchRenderable);
			break;
		}
	}

	void MapRegion::removeAllSurfacePatchRenderablesForLod(PolyVox::uint8_t uLodLevel)
	{
		std::list<SurfacePatchRenderable*>* pList;
		switch(uLodLevel)
		{
			case 0:
			pList = &m_listSurfacePatchRenderablesLod0;
			break;
		case 1:
			pList = &m_listSurfacePatchRenderablesLod1;
			break;
		case 2:
			pList = &m_listSurfacePatchRenderablesLod2;
			break;
		}

		for(std::list<SurfacePatchRenderable*>::iterator iter = pList->begin(); iter != pList->end(); iter++)
		{
			if(m_pOgreSceneNode != 0)
			{
				m_pOgreSceneNode->detachObject(*iter);
			}
			m_pParentMap->m_pOgreSceneManager->destroyMovableObject(*iter);
		}

		pList->clear();
	}

	void MapRegion::setLodLevelToUse(PolyVox::uint8_t uLodLevel)
	{
		for(std::list<SurfacePatchRenderable*>::iterator iter = m_listSurfacePatchRenderablesLod0.begin(); iter != m_listSurfacePatchRenderablesLod0.end(); iter++)
		{
			(*iter)->setVisible(uLodLevel == 0);
		}

		for(std::list<SurfacePatchRenderable*>::iterator iter = m_listSurfacePatchRenderablesLod1.begin(); iter != m_listSurfacePatchRenderablesLod1.end(); iter++)
		{
			(*iter)->setVisible(uLodLevel == 1);
		}

		for(std::list<SurfacePatchRenderable*>::iterator iter = m_listSurfacePatchRenderablesLod2.begin(); iter != m_listSurfacePatchRenderablesLod2.end(); iter++)
		{
			(*iter)->setVisible(uLodLevel == 2);
		}
	}

	void MapRegion::update(IndexedSurfacePatch* ispNew)
	{
		if(qApp->settings()->value("Physics/SimulatePhysics", false).toBool())
		{		
			setPhysicsData(Ogre::Vector3(m_v3dPos.getX(),m_v3dPos.getY(),m_v3dPos.getZ()), *ispNew);
		}
	}

	void MapRegion::setPhysicsData(Ogre::Vector3 pos, const IndexedSurfacePatch& isp)
	{
		const std::string& strName = makeUniqueName("RB");

		destroyPhysicsData();

		if(isp.isEmpty())
		{
			return;
		}

		const float      gDynamicBodyRestitution = 0.6f;
		const float      gDynamicBodyFriction    = 0.6f;
		const float      gDynamicBodyMass        = 1.0f;

		/*if(mTriMesh == 0)
		{*/
			mTriMesh = new btTriangleMesh();
			copyISPToTriangleMesh(isp, mTriMesh);   
		/*}
		else
		{
			updateTriangleMeshWithNewISP(isp, mTriMesh);
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

			mBody->getWorldTransform().setOrigin(btVector3(pos.x, pos.y, pos.z));
			mBody->getWorldTransform().setRotation(btQuaternion(0, 0, 0, 1));

			m_pParentMap->m_pOgreBulletWorld->getBulletDynamicsWorld()->addRigidBody(mBody);
		/*}
		else
		{
			mBody->setCollisionShape(mShape);
		}*/
	}

	void MapRegion::destroyPhysicsData(void)
	{	
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
	}

	std::string MapRegion::makeUniqueName(const std::string& strBase)
	{
		std::stringstream ssName;
		ssName << strBase << " " << ++m_iNameGen;
		return ssName.str();
	}

	void MapRegion::copyISPToTriangleMesh(const PolyVox::IndexedSurfacePatch& isp, btTriangleMesh* triMesh)
	{
		btVector3    vecBulletVertices[3];

		std::vector<PolyVox::uint32_t>::const_iterator indicesIter = isp.getIndices().begin();
		while(indicesIter != isp.getIndices().end())
		{
			for (unsigned int v = 0; v < 3; ++v)
			{
				PolyVox::uint32_t uIndex = *indicesIter;
				const SurfaceVertex& vertex = isp.getVertices()[uIndex];
				const Vector3DFloat& vertexPos = vertex.getPosition();

				vecBulletVertices[v][0] = vertex.getPosition().getX();
				vecBulletVertices[v][1] = vertex.getPosition().getY();
				vecBulletVertices[v][2] = vertex.getPosition().getZ();

				++indicesIter;
			}

			mTriMesh->addTriangle(vecBulletVertices[0], vecBulletVertices[1], vecBulletVertices[2]);
		}
	}

	void MapRegion::updateTriangleMeshWithNewISP(const PolyVox::IndexedSurfacePatch& isp, btTriangleMesh* triMesh)
	{
		mTriMesh->preallocateVertices(isp.getVertices().size());
		mTriMesh->preallocateIndices(isp.getIndices().size());
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

		LogManager::getSingleton().logMessage("Vertex Stride is " + Ogre::StringConverter::toString(stride));
		LogManager::getSingleton().logMessage("Index Stride is " + Ogre::StringConverter::toString(indexstride));

		float* vertexBaseAsFloat = reinterpret_cast<float*>(vertexbase);
		std::vector<SurfaceVertex>::const_iterator verticesIter = isp.getVertices().begin();
		//LogManager::getSingleton().logMessage("copying vertices");
		int ct = 0;
		while(verticesIter != isp.getVertices().end())
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
		std::vector<PolyVox::uint32_t>::const_iterator indicesIter = isp.getIndices().begin();
		while(indicesIter != isp.getIndices().end())
		{
			*indexBaseAsInt = (int)(*indicesIter);
			++indexBaseAsInt;
			++indicesIter;
		}

		//LogManager::getSingleton().logMessage("unlocking");

		mTriMesh->unLockVertexBase(subpart);
	}
}