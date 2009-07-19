#include "SurfaceExtractorThread.h"

#include "MultiThreadedSurfaceExtractor.h"
#include "SurfaceExtractorTaskData.h"

#include "GradientEstimators.h"
#include "IndexedSurfacePatch.h"

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

			//taskData.m_ispResult->generateAveragedFaceNormals(true);
			computeNormalsForVertices(m_pVolData, *(taskData.m_ispResult.get()), SOBEL);
			//taskData.m_ispResult->smooth(0.1f, 5);
			//taskData.m_ispResult->smooth(0.1f, 5);
			//taskData.m_ispResult->smooth(0.1f, 5);

			m_pParentMTSE->pushResult(taskData);
		}
	}
}
