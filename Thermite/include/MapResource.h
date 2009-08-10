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

#ifndef __MapResource_H__
#define __MapResource_H__

#include "Map.h"

#include "OgrePrerequisites.h"
#include "OgreSharedPtr.h"

#include "PolyVoxForwardDeclarations.h"
#include "PolyVoxImpl/TypeDef.h"

#include <OgreResourceManager.h>

namespace Thermite
{
	class MapResource : public Ogre::Resource
	{
	public:		
		MapResource (Ogre::ResourceManager *creator, const Ogre::String &name, 
			Ogre::ResourceHandle handle, const Ogre::String &group, bool isManual = false, 
			Ogre::ManualResourceLoader *loader = 0);
		~MapResource();	

		ThermiteGameLogic* m_pThermiteGameLogic; //Nasty hack to allow progress monitoring
		Ogre::SceneManager* m_pOgreSceneManager;

	protected:

		// must implement these from the Ogre::Resource interface
		void loadImpl ();
		void unloadImpl ();
		size_t calculateSize () const;

	public:
		Map* m_pMap;
	};

	class MapResourcePtr : public Ogre::SharedPtr<MapResource> 
	{
	public:
		MapResourcePtr () : Ogre::SharedPtr<MapResource> () {}
		explicit MapResourcePtr (MapResource *rep) : Ogre::SharedPtr<MapResource> (rep) {}
		MapResourcePtr (const MapResourcePtr &r) : Ogre::SharedPtr<MapResource> (r) {} 
		MapResourcePtr (const Ogre::ResourcePtr &r) : Ogre::SharedPtr<MapResource> ()
		{
			// lock & copy other mutex pointer
			OGRE_LOCK_MUTEX (*r.OGRE_AUTO_MUTEX_NAME)
				OGRE_COPY_AUTO_SHARED_MUTEX (r.OGRE_AUTO_MUTEX_NAME)
				pRep = static_cast<MapResource*> (r.getPointer ());
			pUseCount = r.useCountPointer ();
			if (pUseCount)
			{
				++ (*pUseCount);
			}
		}

		/// Operator used to convert a ResourcePtr to a VolumeResourcePtr
		MapResourcePtr& operator=(const Ogre::ResourcePtr& r)
		{
			if (pRep == static_cast<MapResource*> (r.getPointer ()))
				return *this;
			release ();
			// lock & copy other mutex pointer
			OGRE_LOCK_MUTEX (*r.OGRE_AUTO_MUTEX_NAME)
				OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
				pRep = static_cast<MapResource*> (r.getPointer());
			pUseCount = r.useCountPointer ();
			if (pUseCount)
			{
				++ (*pUseCount);
			}
			return *this;
		}
	};
}

#endif
