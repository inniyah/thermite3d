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
	while(true)
	{
		m_pParentMTSE->m_noOfTasksAvailable->acquire();

		m_pParentMTSE->m_mutexPendingTasks->lock();
		SurfaceExtractorTaskData taskData = m_pParentMTSE->m_queuePendingTasks.front();
		m_pParentMTSE->m_queuePendingTasks.pop();
		m_pParentMTSE->m_mutexPendingTasks->unlock();

		m_pSurfaceExtractor->setLodLevel(taskData.m_uLodLevel);

		taskData.m_ispResult = m_pSurfaceExtractor->extractSurfaceForRegion(taskData.m_regToProcess);

		m_pParentMTSE->m_mutexCompletedTasks->lock();
		m_pParentMTSE->m_listCompletedTasks.push_back(taskData);
		m_pParentMTSE->m_mutexCompletedTasks->unlock();

		m_pParentMTSE->m_noOfResultsAvailable->release();
	}

	m_pParentMTSE->m_bFinished = true;
}
