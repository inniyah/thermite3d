#include "SurfaceExtractorThread.h"

#include "MultiThreadedSurfaceExtractor.h"

using namespace PolyVox;

SurfaceExtractorThread::SurfaceExtractorThread(MultiThreadedSurfaceExtractor* pParentMTSE)
:m_pParentMTSE(pParentMTSE)
{
	m_pSurfaceExtractor = new SurfaceExtractor(*(m_pParentMTSE->m_pVolData));
}

void SurfaceExtractorThread::run(void)
{
	while(m_pParentMTSE->m_queuePendingTasks.empty() == false)
	{
		TaskData taskData = m_pParentMTSE->m_queuePendingTasks.front();
		m_pParentMTSE->m_queuePendingTasks.pop();

		m_pSurfaceExtractor->setLodLevel(taskData.m_uLodLevel);

		POLYVOX_SHARED_PTR<PolyVox::IndexedSurfacePatch> temp = m_pSurfaceExtractor->extractSurfaceForRegion(taskData.m_regToProcess);
		m_pParentMTSE->m_listTemp.push_back(temp);
		taskData.m_ispResult = temp.get();

		m_pParentMTSE->m_listCompletedTasks.push_back(taskData);
	}

	m_pParentMTSE->m_bFinished = true;
}
