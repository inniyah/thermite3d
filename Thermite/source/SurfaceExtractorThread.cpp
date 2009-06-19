#include "SurfaceExtractorThread.h"

#include "MultiThreadedSurfaceExtractor.h"
#include "SurfaceExtractorTaskData.h"

using namespace PolyVox;

namespace Thermite
{
	SurfaceExtractorThread::SurfaceExtractorThread(MultiThreadedSurfaceExtractor* pParentMTSE, PolyVox::Volume<PolyVox::uint8_t>* pVolData)
	:m_pParentMTSE(pParentMTSE)
	,m_pVolData(pVolData)
	{
		m_pSurfaceExtractor = new SurfaceExtractor(*(pVolData));
	}

	SurfaceExtractorThread::~SurfaceExtractorThread()
	{
		delete m_pSurfaceExtractor;
	}

	void SurfaceExtractorThread::run(void)
	{
		while(true)
		{
			SurfaceExtractorTaskData taskData = m_pParentMTSE->popTask();
			m_pSurfaceExtractor->setLodLevel(taskData.m_uLodLevel);
			taskData.m_ispResult = m_pSurfaceExtractor->extractSurfaceForRegion(taskData.m_regToProcess);
			m_pParentMTSE->pushResult(taskData);
		}
	}
}
