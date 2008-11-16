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

#ifndef __VOLUMESERIALIZER_H__
#define __VOLUMESERIALIZER_H__

#include "DataStreamAdapter.h"

#include <OgreSerializer.h>

#include "PolyVoxCore/PolyVoxForwardDeclarations.h"

#include <boost/cstdint.hpp>

namespace Ogre
{
	class VolumeSerializer : public Ogre::Serializer
	{
	public:
		VolumeSerializer ();
		virtual ~VolumeSerializer ();

		//void exportVolume (const Volume *pText, const Ogre::String &fileName);
		void importVolume (Ogre::DataStreamPtr &stream, PolyVox::BlockVolume<boost::uint8_t> **pDest);
	};
}

#endif
