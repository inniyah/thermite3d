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

#ifndef __THERMITE_SURFACEEXTRACTORTASKDATA_H__
#define __THERMITE_SURFACEEXTRACTORTASKDATA_H__

#include "SurfaceExtractorThread.h"

class SurfaceExtractorTaskData
{
	//For writing the result and reading the priority without exposing function
	friend class SurfaceExtractorThread;

public:
	SurfaceExtractorTaskData(void);
	SurfaceExtractorTaskData(PolyVox::Region regToProcess, PolyVox::uint8_t uLodLevel, PolyVox::uint32_t uPriority = 0);

	PolyVox::uint8_t getLodLevel(void) const;
	PolyVox::Region getRegion(void) const;
	PolyVox::uint32_t getPriority(void) const;
	POLYVOX_SHARED_PTR<PolyVox::IndexedSurfacePatch> getIndexedSurfacePatch(void) const;

	void setLodLevel(PolyVox::uint8_t uLodLevel);
	void setPriority(PolyVox::uint32_t uPriority);
	void setRegion(const PolyVox::Region& regToProcess);

private:
	PolyVox::uint8_t m_uLodLevel;
	PolyVox::uint32_t m_uPriority;
	PolyVox::Region m_regToProcess;
	POLYVOX_SHARED_PTR<PolyVox::IndexedSurfacePatch> m_ispResult;
};

#endif