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
	bool finished;
	while(true)
	{
		m_pParentMTSE->m_mutexPendingTasks->lock();
		finished = m_pParentMTSE->m_queuePendingTasks.empty();
		if(finished)
		{
			m_pParentMTSE->m_mutexPendingTasks->unlock();
			this->msleep(10);
		}
		else
		{
			TaskData taskData = m_pParentMTSE->m_queuePendingTasks.front();
			m_pParentMTSE->m_queuePendingTasks.pop();
			m_pParentMTSE->m_mutexPendingTasks->unlock();

			m_pSurfaceExtractor->setLodLevel(taskData.m_uLodLevel);

			taskData.m_ispResult = m_pSurfaceExtractor->extractSurfaceForRegion(taskData.m_regToProcess);

			m_pParentMTSE->m_mutexCompletedTasks->lock();
			m_pParentMTSE->m_listCompletedTasks.push_back(taskData);
			m_pParentMTSE->m_mutexCompletedTasks->unlock();
		}
	}

	m_pParentMTSE->m_bFinished = true;
}
