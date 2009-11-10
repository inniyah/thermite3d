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

#include "DataStreamWrapper.h"
#include "VolumeManager.h"
#include "VolumeResource.h"
#include "VolumeSampler.h"

#include "OgreVector3.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"

#include <iostream> //FIXME - remove this...

using namespace PolyVox;

namespace Thermite
{
	VolumeResource::VolumeResource(	Ogre::ResourceManager* creator, const Ogre::String &name, 
									Ogre::ResourceHandle handle, const Ogre::String &group, bool isManual, 
									Ogre::ManualResourceLoader *loader)
		:Ogre::Resource (creator, name, handle, group, isManual, loader)
	{		
		createParamDictionary ("Volume");
	}

	VolumeResource::~VolumeResource()
	{
		unload ();
	}	

	void VolumeResource::loadImpl ()
	{
		Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton ().openResource (mName, mGroup, true, this);
		std::istream stdStream(new DataStreamWrapper(stream)); 
		m_pVolume = loadVolumeRle(stdStream, VolumeManager::getSingletonPtr()->m_pProgressListener);

		m_pVolume->tidyUpMemory();
	}

	void VolumeResource::unloadImpl ()
	{
		//Clear the pointer
		m_pVolume = POLYVOX_SHARED_PTR< PolyVox::Volume<PolyVox::uint8_t> >();
	}

	size_t VolumeResource::calculateSize () const
	{
		//NOTE - I don't really know what this function is for, so am therefore
		//a bit vague on how to implement it. But here's my best guess...
		return m_pVolume->getWidth() * m_pVolume->getHeight() * m_pVolume->getDepth();
	}

	PolyVox::Volume<PolyVox::uint8_t>* VolumeResource::getVolume(void)
	{
		return m_pVolume.get();
	}
}
