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

#include "MapManager.h"

#include "OgreLogManager.h" //FIXME - shouldn't realy need this in this class?'

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
		Ogre::LogManager::getSingleton().logMessage("DAVID - calling getByName");
		MapResourcePtr textf = getByName (name);
		Ogre::LogManager::getSingleton().logMessage("DAVID - done getByName");

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
		mapResource->m_pThermiteGameLogic = m_pThermiteGameLogic;
		mapResource->m_pOgreSceneManager = m_pOgreSceneManager;
		return mapResource;
	}
}