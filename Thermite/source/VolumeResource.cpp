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
