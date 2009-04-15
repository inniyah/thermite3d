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

#ifndef __TimeStampedSurfacePatchCache_H__
#define __TimeStampedSurfacePatchCache_H__

#include "IndexedSurfacePatch.h"
#include "Vector.h"

#include "VolumeChangeTracker.h"

#include <map>

class TimeStampedSurfacePatchCache
{
private:
	TimeStampedSurfacePatchCache();
	~TimeStampedSurfacePatchCache();

public:
	static TimeStampedSurfacePatchCache* getInstance();
	PolyVox::IndexedSurfacePatch* getIndexedSurfacePatch(PolyVox::Vector3DInt32 position, PolyVox::uint8_t lod);

public:
	std::map<PolyVox::Vector3DInt32, PolyVox::IndexedSurfacePatch*> m_mapIsps[3];

	PolyVox::VolumeChangeTracker* m_vctTracker;

	static TimeStampedSurfacePatchCache* m_pInstance;
};

#endif