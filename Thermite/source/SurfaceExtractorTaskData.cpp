#include "SurfaceExtractorTaskData.h"

using namespace PolyVox;

SurfaceExtractorTaskData::SurfaceExtractorTaskData()
{
}

SurfaceExtractorTaskData::SurfaceExtractorTaskData(Region regToProcess, uint8_t uLodLevel)
:m_uLodLevel(uLodLevel)
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