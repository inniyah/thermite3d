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

#ifndef __SurfacePatchRenderable_H__
#define __SurfacePatchRenderable_H__

#include "Ogre.h"
#include <vector>

#include "PolyVoxCore/IndexedSurfacePatch.h"

//IDEA - If profiling identifies this class as a bottleneck, we could implement a memory pooling system.
//All buffers could be powers of two, and we get the smallest one which is big enough for our needs.
//See http://www.ogre3d.org/wiki/index.php/DynamicGrowingBuffers
class SurfacePatchRenderable : public Ogre::MovableObject, public Ogre::Renderable
{
public:
	SurfacePatchRenderable(const Ogre::String& strName);
	~SurfacePatchRenderable(void);

	const Ogre::AxisAlignedBox& getBoundingBox(void) const;
	Ogre::Real getBoundingRadius(void) const;
	const Ogre::LightList& getLights(void) const;
	const Ogre::MaterialPtr& getMaterial(void) const;
	const Ogre::String& getMovableType(void) const;
	void getRenderOperation(Ogre::RenderOperation& op);
	Ogre::Real getSquaredViewDepth(const Ogre::Camera *cam) const;
	const Ogre::Quaternion &getWorldOrientation(void) const;
	const Ogre::Vector3 &getWorldPosition(void) const;
	void getWorldTransforms( Ogre::Matrix4* xform ) const;	

	void setBoundingBox( const Ogre::AxisAlignedBox& box );
	void setMaterial( const Ogre::String& matName );
	void setWorldTransform( const Ogre::Matrix4& xform );
	
	virtual void _notifyCurrentCamera( Ogre::Camera* cam );
	virtual void _updateRenderQueue(Ogre::RenderQueue* queue);
	void visitRenderables(Ogre::Renderable::Visitor* visitor, bool debugRenderables = false);

protected:
	Ogre::RenderOperation* m_RenderOp;
	Ogre::Matrix4 m_matWorldTransform;
	Ogre::Camera *m_pCamera;
	Ogre::AxisAlignedBox mBox;
	Ogre::String m_strMatName;
    Ogre::MaterialPtr m_pMaterial;
	Ogre::SceneManager *m_pParentSceneManager;

public:
	Ogre::Vector3 m_v3dPos;
};

/** Factory object for creating Light instances */
class SurfacePatchRenderableFactory : public Ogre::MovableObjectFactory
{
protected:
	Ogre::MovableObject* createInstanceImpl( const Ogre::String& name, const Ogre::NameValuePairList* params);
public:
	SurfacePatchRenderableFactory() {}
	~SurfacePatchRenderableFactory() {}

	static Ogre::String FACTORY_TYPE_NAME;

	const Ogre::String& getType(void) const;
	void destroyInstance( Ogre::MovableObject* obj);  

};

#endif /* __SurfacePatchRenderable_H__ */
