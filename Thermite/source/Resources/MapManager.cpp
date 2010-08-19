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

#include "MapManager.h"

template<> Thermite::MapManager *Ogre::Singleton<Thermite::MapManager>::ms_Singleton = 0;

namespace Thermite
{
	MapManager *MapManager::getSingletonPtr ()
	{
		return ms_Singleton;
	}

	MapManager &MapManager::getSingleton ()
	{  
		assert (ms_Singleton);  
		return (*ms_Singleton);
	}

	MapManager::MapManager ()
	{
		mResourceType = "Map";

		// low, because it will likely reference other resources
		mLoadOrder = 30.0f;

		// this is how we register the ResourceManager with OGRE
		Ogre::ResourceGroupManager::getSingleton ()._registerResourceManager (mResourceType, this);
	}

	MapManager::~MapManager()
	{
		// and this is how we unregister it
		Ogre::ResourceGroupManager::getSingleton ()._unregisterResourceManager (mResourceType);
	}

	MapResourcePtr MapManager::load (const Ogre::String &name, const Ogre::String &group)
	{
		MapResourcePtr textf = getByName (name);

		if (textf.isNull ())
		{
			textf = create (name, group);
		}

		textf->load ();

		return textf;
	}

	Ogre::Resource *MapManager::createImpl (const Ogre::String &name, Ogre::ResourceHandle handle, 
												const Ogre::String &group, bool isManual, Ogre::ManualResourceLoader *loader, 
												const Ogre::NameValuePairList *createParams)
	{
		MapResource* mapResource = new MapResource (this, name, handle, group, isManual, loader);
		return mapResource;
	}
}