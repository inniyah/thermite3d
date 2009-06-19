#include "SurfaceExtractorTaskData.h"

using namespace PolyVox;

namespace Thermite
{
	SurfaceExtractorTaskData::SurfaceExtractorTaskData()
	{
	}

	SurfaceExtractorTaskData::SurfaceExtractorTaskData(Region regToProcess, uint8_t uLodLevel, uint32_t uPriority)
	:m_uLodLevel(uLodLevel)
	,m_uPriority(uPriority)
	,m_regToProcess(regToProcess)
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

	uint32_t SurfaceExtractorTaskData::getPriority(void) const
	{
		return m_uPriority;
	}

	POLYVOX_SHARED_PTR<IndexedSurfacePatch> SurfaceExtractorTaskData::getIndexedSurfacePatch(void) const
	{
		return m_ispResult;
	}

	void SurfaceExtractorTaskData::setLodLevel(uint8_t uLodLevel)
	{
		m_uLodLevel = uLodLevel;
	}

	void SurfaceExtractorTaskData::setPriority(uint32_t uPriority)
	{
		m_uPriority = uPriority;
	}

	void SurfaceExtractorTaskData::setRegion(const Region& regToProcess)
	{
		m_regToProcess = regToProcess;
	}
}