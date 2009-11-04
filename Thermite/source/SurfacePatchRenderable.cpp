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

#include "SurfacePatchRenderable.h"

#include "Application.h"

#include "SurfaceVertex.h"
#include "OgreVertexIndexData.h"

#include <QSettings>

#include <limits>

using namespace PolyVox;
using namespace Ogre;

namespace Thermite
{
#pragma region Constructors/Destructors
	SurfacePatchRenderable::SurfacePatchRenderable(const String& strName)
		:m_RenderOp(0)
	{
		mName = strName;
		m_matWorldTransform = Ogre::Matrix4::IDENTITY;
		m_strMatName = "BaseWhite"; 
		m_pMaterial = Ogre::MaterialManager::getSingleton().getByName("BaseWhite");
		m_pParentSceneManager = NULL;
		mParentNode = NULL;

		//FIXME - use proper values.
		mBox.setExtents(Ogre::Vector3(-1000.0f,-1000.0f,-1000.0f), Ogre::Vector3(1000.0f,1000.0f,1000.0f));
	}

	SurfacePatchRenderable::~SurfacePatchRenderable(void)
	{
		if(m_RenderOp)
		{
			delete m_RenderOp->vertexData;
			delete m_RenderOp->indexData;
			delete m_RenderOp;
		}
	}
#pragma endregion

#pragma region Getters
	const Ogre::AxisAlignedBox& SurfacePatchRenderable::getBoundingBox(void) const
	{
		return mBox;
	}

	Real SurfacePatchRenderable::getBoundingRadius(void) const
	{
		return Math::Sqrt((std::max)(mBox.getMaximum().squaredLength(), mBox.getMinimum().squaredLength()));
	}

	const Ogre::LightList& SurfacePatchRenderable::getLights(void) const
	{
		// Use movable query lights
		return queryLights();
	}

	const Ogre::MaterialPtr& SurfacePatchRenderable::getMaterial(void) const
	{
		return m_pMaterial;
	}

	const String& SurfacePatchRenderable::getMovableType(void) const
	{
		static String movType = "SurfacePatchRenderable";
		return movType;
	}

	void SurfacePatchRenderable::getRenderOperation(Ogre::RenderOperation& op)
	{
		//This doesn't cause a crash when op is null, because in that case this function
		//won't be called as this renderable won't have been added by _updateRenderQueue().
		op = *m_RenderOp;
	}

	Real SurfacePatchRenderable::getSquaredViewDepth(const Camera *cam) const
	{
		Vector3 vMin, vMax, vMid, vDist;
		vMin = mBox.getMinimum();
		vMax = mBox.getMaximum();
		vMid = ((vMin - vMax) * 0.5) + vMin;
		vDist = cam->getDerivedPosition() - vMid;

		return vDist.squaredLength();
	}

	const Quaternion &SurfacePatchRenderable::getWorldOrientation(void) const
	{
		return Quaternion::IDENTITY;
	}

	const Vector3 &SurfacePatchRenderable::getWorldPosition(void) const
	{
		return Vector3::ZERO;
	}

	void SurfacePatchRenderable::getWorldTransforms( Ogre::Matrix4* xform ) const
	{
		*xform = m_matWorldTransform * mParentNode->_getFullTransform();
	}
#pragma endregion

#pragma region Setters
	void SurfacePatchRenderable::setBoundingBox( const Ogre::AxisAlignedBox& box )
	{
		mBox = box;
	}

	void SurfacePatchRenderable::setMaterial( const Ogre::String& matName )
	{
		m_strMatName = matName;
		m_pMaterial = Ogre::MaterialManager::getSingleton().getByName(m_strMatName);
		if (m_pMaterial.isNull())
			OGRE_EXCEPT( Ogre::Exception::ERR_ITEM_NOT_FOUND, "Could not find material " + m_strMatName,
			"SurfacePatchRenderable::setMaterial" );

		// Won't load twice anyway
		m_pMaterial->load();
	}

	void SurfacePatchRenderable::setWorldTransform( const Ogre::Matrix4& xform )
	{
		m_matWorldTransform = xform;
	}
#pragma endregion

#pragma region Other

	void SurfacePatchRenderable::_updateRenderQueue(RenderQueue* queue)
	{
		if(m_RenderOp)
		{
			//Single material patches get rendered first, so the multi material
			//patches can be added afterwards using additive blending.
			if(isSingleMaterial())
			{
				queue->addRenderable( this, mRenderQueueID, RENDER_QUEUE_MAIN); 
			}
			else
			{
				queue->addRenderable( this, mRenderQueueID, RENDER_QUEUE_6); 
			}
		}
	}

	void SurfacePatchRenderable::visitRenderables(Ogre::Renderable::Visitor* visitor, bool debugRenderables)
	{
		visitor->visit(this, 0, false);
	}

	Real* SurfacePatchRenderable::addVertex(const SurfaceVertex& vertex, float alpha, Real* prPos)
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

	void SurfacePatchRenderable::buildRenderOperationFrom(IndexedSurfacePatch& isp, bool bSingleMaterial)
	{
		if(isp.isEmpty())
		{
			m_RenderOp = 0;
			return;
		}

		if((!bSingleMaterial) && (isp.getNoOfNonUniformTrianges() == 0))
		{
			m_RenderOp = 0;
			return;
		}

		m_bIsSingleMaterial = bSingleMaterial;

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

		//The '3 * 3' in the following expressions comes from the fact that when we encounter a non uniform
		//triangle we make it degenerate and add three new ones. That is an increase of nine vertices.
		if(bSingleMaterial)
		{
			renderOperation->vertexData->vertexCount = (vecVertices.size()) + (isp.getNoOfNonUniformTrianges() * 3);		
			renderOperation->indexData->indexCount = vecIndices.size();	
		}
		else
		{
			renderOperation->vertexData->vertexCount = (isp.getNoOfNonUniformTrianges() * 3 * 3);		
			renderOperation->indexData->indexCount = (isp.getNoOfNonUniformTrianges() * 3 * 3);	
		}
		
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
			HardwareIndexBuffer::IT_32BIT, // type of index
			renderOperation->indexData->indexCount, // number of indexes
			HardwareBuffer::HBU_STATIC_WRITE_ONLY, // usage
			false); // no shadow buffer	

		renderOperation->indexData->indexBuffer = ibuf;	

		// Drawing stuff
		Vector3 vaabMin(std::numeric_limits<Real>::max(),std::numeric_limits<Real>::max(),std::numeric_limits<Real>::max());
		Vector3 vaabMax(0.0,0.0,0.0);
		
		Real *prPos = static_cast<Real*>(vbuf->lock(HardwareBuffer::HBL_DISCARD));

		if(bSingleMaterial)
		{
			for(std::vector<SurfaceVertex>::const_iterator vertexIter = vecVertices.begin(); vertexIter != vecVertices.end(); ++vertexIter)
			{			
				prPos = addVertex(*vertexIter, 1.0f, prPos);	
				
			}
		}

		if(bSingleMaterial)
		{
			unsigned long* pIdx = static_cast<unsigned long*>(ibuf->lock(HardwareBuffer::HBL_DISCARD));
			unsigned long newVertexIndex = vecVertices.size();
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
					/**pIdx = 0;
					pIdx++;
					*pIdx = 0;
					pIdx++;
					*pIdx = 0;
					pIdx++;*/

					//Construct new vertices
					SurfaceVertex vert0 = vecVertices[vecIndices[i+0]];
					SurfaceVertex vert1 = vecVertices[vecIndices[i+1]];
					SurfaceVertex vert2 = vecVertices[vecIndices[i+2]];

					/*float mat0 = vert0.getMaterial();
					float mat1 = vert1.getMaterial();
					float mat2 = vert2.getMaterial();*/

					vert0.setMaterial(0);
					vert1.setMaterial(0);
					vert2.setMaterial(0);

					/*prPos = addVertex(vert0, 1.0, prPos);
					prPos = addVertex(vert1, 0.0, prPos);
					prPos = addVertex(vert2, 0.0, prPos);*/

					prPos = addVertex(vert0, 1.0, prPos);
					prPos = addVertex(vert1, 1.0, prPos);
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

					//Construct new vertices
					/*vert0.setMaterial(mat1);
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
					newVertexIndex++;*/
				}
			}
		}
		else
		{
			unsigned long* pIdx = static_cast<unsigned long*>(ibuf->lock(HardwareBuffer::HBL_DISCARD));
			unsigned long newVertexIndex = 0; //vecVertices.size();
			for(int i = 0; i < vecIndices.size() - 2; i += 3)
			{
				if((vecVertices[vecIndices[i]].getMaterial() == vecVertices[vecIndices[i+1]].getMaterial()) && (vecVertices[vecIndices[i]].getMaterial() == vecVertices[vecIndices[i+2]].getMaterial()))
				{
					/**pIdx = vecIndices[i];
					pIdx++;
					*pIdx = vecIndices[i+1];
					pIdx++;
					*pIdx = vecIndices[i+2];
					pIdx++;*/
				}	
				else
				{
					//Make the non uniform triangle degenerate
					/**pIdx = 0;
					pIdx++;
					*pIdx = 0;
					pIdx++;
					*pIdx = 0;
					pIdx++;*/

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

					if(isp.m_mapUsedMaterials.find(mat0) != isp.m_mapUsedMaterials.end())
					{
						prPos = addVertex(vert0, 1.0, prPos);
					}
					else
					{
						prPos = addVertex(vert0, 0.0, prPos);
					}
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
					if(isp.m_mapUsedMaterials.find(mat1) != isp.m_mapUsedMaterials.end())
					{
						prPos = addVertex(vert1, 1.0, prPos);
					}
					else
					{
						prPos = addVertex(vert1, 0.0, prPos);
					}
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
					if(isp.m_mapUsedMaterials.find(mat2) != isp.m_mapUsedMaterials.end())
					{
						prPos = addVertex(vert2, 1.0, prPos);
					}
					else
					{
						prPos = addVertex(vert2, 0.0, prPos);
					}

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
		}

		ibuf->unlock();
		vbuf->unlock();

		//This function is extreamly slow.
		//renderOperation->indexData->optimiseVertexCacheTriList();

		m_RenderOp = renderOperation;
	}

	bool SurfacePatchRenderable::isSingleMaterial(void)
	{
		return m_bIsSingleMaterial;
	}

#pragma endregion

	//-----------------------------------------------------------------------
	String SurfacePatchRenderableFactory::FACTORY_TYPE_NAME = "SurfacePatchRenderable";
	//-----------------------------------------------------------------------
	const String& SurfacePatchRenderableFactory::getType(void) const
	{
		return FACTORY_TYPE_NAME;
	}
	//-----------------------------------------------------------------------
	MovableObject* SurfacePatchRenderableFactory::createInstanceImpl(const String& name, const NameValuePairList* params)
	{
		return new SurfacePatchRenderable(name);
	}
	//-----------------------------------------------------------------------
	void SurfacePatchRenderableFactory::destroyInstance( MovableObject* obj)
	{
		delete obj;
	}
}
