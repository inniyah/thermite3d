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

#ifndef __THERMITE_SURFACEEXTRACTORTASKDATA_H__
#define __THERMITE_SURFACEEXTRACTORTASKDATA_H__

#include <PolyVoxForwardDeclarations.h>
#include <Region.h>

namespace Thermite
{
	class SurfaceExtractorTaskData
	{
	public:
		SurfaceExtractorTaskData(void);
		SurfaceExtractorTaskData(PolyVox::Region regToProcess, PolyVox::uint32_t uTimeStamp);

		PolyVox::Region getRegion(void) const;
		POLYVOX_SHARED_PTR<PolyVox::IndexedSurfacePatch> getIndexedSurfacePatch(void) const;

		void setRegion(const PolyVox::Region& regToProcess);

	public:
		PolyVox::Region m_regToProcess;
		POLYVOX_SHARED_PTR<PolyVox::IndexedSurfacePatch> m_ispResult;
		PolyVox::uint32_t m_uTimeStamp;
	};
}

#endif