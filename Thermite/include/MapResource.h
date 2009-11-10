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

#ifndef __MapResource_H__
#define __MapResource_H__

#include "Map.h"

#include "OgrePrerequisites.h"
#include "OgreSharedPtr.h"

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
