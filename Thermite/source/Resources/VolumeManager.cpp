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

#include "VolumeManager.h"

#include "OgreLogManager.h" //FIXME - shouldn't realy need this in this class?'

template<> Thermite::VolumeManager *Ogre::Singleton<Thermite::VolumeManager>::msSingleton = 0;

namespace Thermite
{
	VolumeManager *VolumeManager::getSingletonPtr ()
	{
		return msSingleton;
	}

	VolumeManager &VolumeManager::getSingleton ()
	{  
		assert (msSingleton);  
		return (*msSingleton);
	}

	VolumeManager::VolumeManager ()
	:m_pProgressListener(0)
	{
		mResourceType = "Volume";

		// low, because it will likely reference other resources
		mLoadOrder = 30.0f;

		// this is how we register the ResourceManager with OGRE
		Ogre::ResourceGroupManager::getSingleton ()._registerResourceManager (mResourceType, this);
	}

	VolumeManager::~VolumeManager()
	{
		// and this is how we unregister it
		Ogre::ResourceGroupManager::getSingleton ()._unregisterResourceManager (mResourceType);
	}

	VolumeResourcePtr VolumeManager::create (const Ogre::String& name, const Ogre::String& group,
					bool isManual, Ogre::ManualResourceLoader* loader,
					const Ogre::NameValuePairList* createParams)
	{
		return std::static_pointer_cast<VolumeResource>(createResource(name,group,isManual,loader,createParams));
	}

	VolumeResourcePtr VolumeManager::getByName(const Ogre::String& name, const Ogre::String& groupName)
	{
		return std::static_pointer_cast<VolumeResource>(getResourceByName(name, groupName));
	}

	VolumeResourcePtr VolumeManager::load (const Ogre::String &name, const Ogre::String &group)
	{
		Ogre::LogManager::getSingleton().logMessage("DAVID - calling getByName");
		VolumeResourcePtr textf = getByName (name);
		Ogre::LogManager::getSingleton().logMessage("DAVID - done getByName");

		if (textf)
		{
			textf = create (name, group);
		}

		textf->load ();

		return textf;
	}

	Ogre::Resource *VolumeManager::createImpl (const Ogre::String &name, Ogre::ResourceHandle handle, 
												const Ogre::String &group, bool isManual, Ogre::ManualResourceLoader *loader, 
												const Ogre::NameValuePairList *createParams)
	{
		return new VolumeResource (this, name, handle, group, isManual, loader);
	}
}