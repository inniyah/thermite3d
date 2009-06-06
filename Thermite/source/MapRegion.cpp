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

unsigned long MapRegion::m_iNameGen = 0;

MapRegion::MapRegion(Map* pParentMap, PolyVox::Vector3DInt16 v3dPos)
:m_pOgreSceneNode(0)
,m_pSurfacePatchRenderable(0)
,m_pParentMap(pParentMap)
,mBody(0)
,mTriMesh(0)
,mShape(0)
{
	m_v3dPos = v3dPos;

	const std::string& strNodeName = makeUniqueName("SN");
	m_pOgreSceneNode = m_pParentMap->m_pOgreSceneManager->getRootSceneNode()->createChildSceneNode(strNodeName);
	m_pOgreSceneNode->setPosition(Ogre::Vector3(m_v3dPos.getX(),m_v3dPos.getY(),m_v3dPos.getZ()));

	const std::string& strSprName = makeUniqueName("SPR");
	m_pSurfacePatchRenderable = dynamic_cast<SurfacePatchRenderable*>(m_pParentMap->m_pOgreSceneManager->createMovableObject(strSprName, SurfacePatchRenderableFactory::FACTORY_TYPE_NAME));
	m_pSurfacePatchRenderable->setMaterial("ShadowMapReceiverForWorldMaterial");
	m_pSurfacePatchRenderable->setCastShadows(qApp->settings()->value("Shadows/EnableShadows", false).toBool());
	m_pOgreSceneNode->attachObject(m_pSurfacePatchRenderable);
	m_pSurfacePatchRenderable->m_v3dPos = m_pOgreSceneNode->getPosition();
	m_pSurfacePatchRenderable->pParent = this;

	int regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();
	AxisAlignedBox aabb(Vector3(0.0f,0.0f,0.0f), Vector3(regionSideLength, regionSideLength, regionSideLength));
	m_pSurfacePatchRenderable->setBoundingBox(aabb);
}

MapRegion::~MapRegion()
{
	if(m_pSurfacePatchRenderable != 0)
	{
		if(m_pOgreSceneNode != 0)
		{
			m_pOgreSceneNode->detachObject(m_pSurfacePatchRenderable);
		}
		m_pParentMap->m_pOgreSceneManager->destroyMovableObject(m_pSurfacePatchRenderable);
		m_pSurfacePatchRenderable = 0;
	}

	if(m_pOgreSceneNode != 0)
	{
		m_pParentMap->m_pOgreSceneManager->getRootSceneNode()->removeAndDestroyChild(m_pOgreSceneNode->getName());
		m_pOgreSceneNode = 0;
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

Real* MapRegion::addVertex(const SurfaceVertex& vertex, float alpha, Real* prPos)
{
	*prPos++ = vertex.getPosition().getX();
	*prPos++ = vertex.getPosition().getY();
	*prPos++ = vertex.getPosition().getZ();

	*prPos++ = vertex.getNormal().getX();
	*prPos++ = vertex.getNormal().getY();
	*prPos++ = vertex.getNormal().getZ();

	*prPos++ = vertex.getMaterial();

	*prPos++ = alpha;

	return prPos;
}

RenderOperation* MapRegion::buildRenderOperationFrom(IndexedSurfacePatch& isp)
{
	if(isp.isEmpty())
	{
		return 0;
	}

	RenderOperation* renderOperation = new RenderOperation();

	//Set up what we can of the vertex data
	renderOperation->vertexData = new VertexData();
	renderOperation->vertexData->vertexStart = 0;
	renderOperation->vertexData->vertexCount = 0;
	renderOperation->operationType = RenderOperation::OT_TRIANGLE_LIST;

	//Set up what we can of the index data
	renderOperation->indexData = new IndexData();
	renderOperation->useIndexes = true;
	renderOperation->indexData->indexStart = 0;
	renderOperation->indexData->indexCount = 0;

	//Set up the vertex declaration
	VertexDeclaration *decl = renderOperation->vertexData->vertexDeclaration;
	decl->removeAllElements();
	decl->addElement(0, 0, VET_FLOAT3, VES_POSITION);
	decl->addElement(0, 3 * sizeof(float), VET_FLOAT3, VES_NORMAL);
	decl->addElement(0, 6 * sizeof(float), VET_FLOAT2, VES_TEXTURE_COORDINATES);

	const std::vector<SurfaceVertex>& vecVertices = isp.getVertices();
	const std::vector<PolyVox::uint32_t>& vecIndices = isp.getIndices();

	//The '9' in the following expressions comes from the fact that when we encounter a non uniform
	//triangle we make it degenerate and add three new ones. That is an increase of nine vertices.
	renderOperation->vertexData->vertexCount = (vecVertices.size()) + (isp.getNoOfNonUniformTrianges() * 9);		
	renderOperation->indexData->indexCount = (vecIndices.size()) + (isp.getNoOfNonUniformTrianges() * 9);	
	
	VertexBufferBinding *bind = renderOperation->vertexData->vertexBufferBinding;

	HardwareVertexBufferSharedPtr vbuf =
		HardwareBufferManager::getSingleton().createVertexBuffer(
		renderOperation->vertexData->vertexDeclaration->getVertexSize(0),
		renderOperation->vertexData->vertexCount,
		HardwareBuffer::HBU_STATIC_WRITE_ONLY,
		false);

	bind->setBinding(0, vbuf);

	HardwareIndexBufferSharedPtr ibuf =
		HardwareBufferManager::getSingleton().createIndexBuffer(
		HardwareIndexBuffer::IT_16BIT, // type of index
		renderOperation->indexData->indexCount, // number of indexes
		HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
		false); // no shadow buffer	

	renderOperation->indexData->indexBuffer = ibuf;	

	// Drawing stuff
	Vector3 vaabMin(std::numeric_limits<Real>::max(),std::numeric_limits<Real>::max(),std::numeric_limits<Real>::max());
	Vector3 vaabMax(0.0,0.0,0.0);
	
	Real *prPos = static_cast<Real*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));

	for(std::vector<SurfaceVertex>::const_iterator vertexIter = vecVertices.begin(); vertexIter != vecVertices.end(); ++vertexIter)
	{			
		prPos = addVertex(*vertexIter, 1.0f, prPos);	
		
	}			

	
	
	unsigned short* pIdx = static_cast<unsigned short*>(ibuf->lock(HardwareBuffer::HBL_DISCARD));
	unsigned short newVertexIndex = vecVertices.size();
	for(int i = 0; i < vecIndices.size() - 2; i += 3)
	{
		if((vecVertices[vecIndices[i]].getMaterial() == vecVertices[vecIndices[i+1]].getMaterial()) && (vecVertices[vecIndices[i]].getMaterial() == vecVertices[vecIndices[i+2]].getMaterial()))
		{
			*pIdx = vecIndices[i];
			pIdx++;
			*pIdx = vecIndices[i+1];
			pIdx++;
			*pIdx = vecIndices[i+2];
			pIdx++;
		}	
		else
		{
			//Make the non uniform triangle degenerate
			*pIdx = 0;
			pIdx++;
			*pIdx = 0;
			pIdx++;
			*pIdx = 0;
			pIdx++;

			//Construct new vertices
			SurfaceVertex vert0 = vecVertices[vecIndices[i+0]];
			SurfaceVertex vert1 = vecVertices[vecIndices[i+1]];
			SurfaceVertex vert2 = vecVertices[vecIndices[i+2]];

			float mat0 = vert0.getMaterial();
			float mat1 = vert1.getMaterial();
			float mat2 = vert2.getMaterial();

			vert0.setMaterial(mat0);
			vert1.setMaterial(mat0);
			vert2.setMaterial(mat0);

			prPos = addVertex(vert0, 1.0, prPos);
			prPos = addVertex(vert1, 0.0, prPos);
			prPos = addVertex(vert2, 0.0, prPos);

			*pIdx = newVertexIndex;
			pIdx++;
			newVertexIndex++;
			*pIdx = newVertexIndex;
			pIdx++;
			newVertexIndex++;
			*pIdx = newVertexIndex;
			pIdx++;
			newVertexIndex++;

			//Construct new vertices
			vert0.setMaterial(mat1);
			vert1.setMaterial(mat1);
			vert2.setMaterial(mat1);

			prPos = addVertex(vert0, 0.0, prPos);
			prPos = addVertex(vert1, 1.0, prPos);
			prPos = addVertex(vert2, 0.0, prPos);

			*pIdx = newVertexIndex;
			pIdx++;
			newVertexIndex++;
			*pIdx = newVertexIndex;
			pIdx++;
			newVertexIndex++;
			*pIdx = newVertexIndex;
			pIdx++;
			newVertexIndex++;

			//Construct new vertices
			vert0.setMaterial(mat2);
			vert1.setMaterial(mat2);
			vert2.setMaterial(mat2);

			prPos = addVertex(vert0, 0.0, prPos);
			prPos = addVertex(vert1, 0.0, prPos);
			prPos = addVertex(vert2, 1.0, prPos);

			*pIdx = newVertexIndex;
			pIdx++;
			newVertexIndex++;
			*pIdx = newVertexIndex;
			pIdx++;
			newVertexIndex++;
			*pIdx = newVertexIndex;
			pIdx++;
			newVertexIndex++;
		}
	}	

	ibuf->unlock();
	vbuf->unlock();	

	return renderOperation;
}