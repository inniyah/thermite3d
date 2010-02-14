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

#include "SurfaceExtractorTaskData.h"

using namespace PolyVox;
using PolyVox::uint8_t;
using PolyVox::uint32_t;


namespace Thermite
{
	SurfaceExtractorTaskData::SurfaceExtractorTaskData()
	{
	}

	SurfaceExtractorTaskData::SurfaceExtractorTaskData(Region regToProcess, uint8_t uLodLevel, PolyVox::uint32_t uTimeStamp)
	:m_uLodLevel(uLodLevel)
	,m_regToProcess(regToProcess)
	,m_uTimeStamp(uTimeStamp)
	{
	}

	uint8_t SurfaceExtractorTaskData::getLodLevel(void) const
	{
		return m_uLodLevel;
	}

	Region SurfaceExtractorTaskData::getRegion(void) const
	{
		return m_regToProcess;
	}

	POLYVOX_SHARED_PTR<IndexedSurfacePatch> SurfaceExtractorTaskData::getIndexedSurfacePatch(void) const
	{
		return m_ispResult;
	}

	void SurfaceExtractorTaskData::setLodLevel(uint8_t uLodLevel)
	{
		m_uLodLevel = uLodLevel;
	}

	void SurfaceExtractorTaskData::setRegion(const Region& regToProcess)
	{
		m_regToProcess = regToProcess;
	}
}