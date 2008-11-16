#include "TimeStampedRenderOperationCache.h"
#include "TimeStampedSurfacePatchCache.h"
#include "SurfacePatchRenderable.h"

#include "PolyVoxCore/SurfaceExtractors.h"

using namespace Ogre;
using namespace PolyVox;
using namespace std;

TimeStampedRenderOperationCache* TimeStampedRenderOperationCache::m_pInstance = 0;

TimeStampedRenderOperationCache::TimeStampedRenderOperationCache()
{
}

TimeStampedRenderOperationCache* TimeStampedRenderOperationCache::getInstance()
{
	if(m_pInstance == 0)
	{
		m_pInstance = new TimeStampedRenderOperationCache();
	}

	return m_pInstance;
}

TimeStampedRenderOperation* TimeStampedRenderOperationCache::getRenderOperation(Vector3DInt32 position, PolyVox::uint8 lod)
{
	//Get the surface patch, creating it if it doesn't exist.
	std::map<Vector3DInt32, TimeStampedRenderOperation*>& mapRenderOps = m_mapRenderOps[lod];

	TimeStampedRenderOperation* renderOpResult;
	std::map<Vector3DInt32, TimeStampedRenderOperation*>::iterator iterMapRenderOp = mapRenderOps.find(position);
	if(iterMapRenderOp == mapRenderOps.end())
	{
		renderOpResult = new TimeStampedRenderOperation();
		mapRenderOps.insert(make_pair(position, renderOpResult));
	}
	else
	{
		renderOpResult = iterMapRenderOp->second;
	}

	//Get the time stamps
	int32 regionTimeStamp = m_vctTracker->getLastModifiedTimeForRegion(position.getX()/POLYVOX_REGION_SIDE_LENGTH,position.getY()/POLYVOX_REGION_SIDE_LENGTH,position.getZ()/POLYVOX_REGION_SIDE_LENGTH);
	int32 renderOpTimeStamp = renderOpResult->m_iTimeStamp;

	if(regionTimeStamp > renderOpTimeStamp) //Need to regenerate render operation
	{

		IndexedSurfacePatch* isp = TimeStampedSurfacePatchCache::getInstance()->getIndexedSurfacePatch(position, lod);
		renderOpResult->m_renderOperation = buildRenderOperationFrom(*isp);
		renderOpResult->m_iTimeStamp = isp->m_iTimeStamp;
	}

	return renderOpResult;
}

Real* TimeStampedRenderOperationCache::addVertex(const SurfaceVertex& vertex, float alpha, Real* prPos)
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

RenderOperation* TimeStampedRenderOperationCache::buildRenderOperationFrom(IndexedSurfacePatch& isp)
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
	const std::vector<PolyVox::uint32>& vecIndices = isp.getIndices();

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