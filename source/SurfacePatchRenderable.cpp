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
#include "TimeStampedRenderOperation.h"
#include "TimeStampedRenderOperationCache.h"

#include "PolyVoxCore/SurfaceVertex.h"
#include "OgreVertexIndexData.h"

#include <QSettings>

#include <limits>

using namespace PolyVox;
using namespace Ogre;

#pragma region Constructors/Destructors
SurfacePatchRenderable::SurfacePatchRenderable(const String& strName)
:m_pCamera(0)
{
	mName = strName;
	m_matWorldTransform = Ogre::Matrix4::IDENTITY;
	m_pCamera = 0;
	m_strMatName = "BaseWhite"; 
	m_pMaterial = Ogre::MaterialManager::getSingleton().getByName("BaseWhite");
	m_pParentSceneManager = NULL;
	mParentNode = NULL;

	//FIXME - use proper values.
	mBox.setExtents(Ogre::Vector3(-1000.0f,-1000.0f,-1000.0f), Ogre::Vector3(1000.0f,1000.0f,1000.0f));
}

SurfacePatchRenderable::~SurfacePatchRenderable(void)
{
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
void SurfacePatchRenderable::_notifyCurrentCamera( Camera* cam )
{
	MovableObject::_notifyCurrentCamera(cam);
	m_pCamera = cam;
}

void SurfacePatchRenderable::_updateRenderQueue(RenderQueue* queue)
{
	Vector3 camPos = m_pCamera->getDerivedPosition();
	float dist = m_v3dPos.distance(camPos);
	if(dist > qApp->settings()->value("Engine/Lod1ToLod2Boundary", 512.0f).toDouble())
	{
		m_RenderOp = TimeStampedRenderOperationCache::getInstance()->getRenderOperation(Vector3DInt32(m_v3dPos.x+0.5,m_v3dPos.y+0.5,m_v3dPos.z+0.5), 2)->m_renderOperation;
	}
	else if(dist > qApp->settings()->value("Engine/Lod0ToLod1Boundary", 256.0f).toDouble())
	{
		m_RenderOp = TimeStampedRenderOperationCache::getInstance()->getRenderOperation(Vector3DInt32(m_v3dPos.x+0.5,m_v3dPos.y+0.5,m_v3dPos.z+0.5), 1)->m_renderOperation;
	}
	else
	{
		m_RenderOp = TimeStampedRenderOperationCache::getInstance()->getRenderOperation(Vector3DInt32(m_v3dPos.x+0.5,m_v3dPos.y+0.5,m_v3dPos.z+0.5), 0)->m_renderOperation;
	}

	if(m_RenderOp)
	{
		queue->addRenderable( this, mRenderQueueID, OGRE_RENDERABLE_DEFAULT_PRIORITY); 
	}
}

void SurfacePatchRenderable::visitRenderables(Ogre::Renderable::Visitor* visitor, bool debugRenderables)
{
	visitor->visit(this, 0, false);
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
