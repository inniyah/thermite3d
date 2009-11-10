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
}

#endif