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

#ifndef __VOLUMEMANAGER_H__
#define __VOLUMEMANAGER_H__

#include <OgreResourceManager.h>
#include "VolumeResource.h"


class VolumeManager : public Ogre::ResourceManager, public Ogre::Singleton<VolumeManager>
{
protected:

	// must implement this from ResourceManager's interface
	Ogre::Resource *createImpl(const Ogre::String &name, Ogre::ResourceHandle handle, 
		const Ogre::String &group, bool isManual, Ogre::ManualResourceLoader *loader, 
		const Ogre::NameValuePairList *createParams);

public:

	VolumeManager ();
	virtual ~VolumeManager ();

	virtual VolumeResourcePtr load (const Ogre::String &name, const Ogre::String &group);

	static VolumeManager &getSingleton ();
	static VolumeManager *getSingletonPtr ();
};

#endif
