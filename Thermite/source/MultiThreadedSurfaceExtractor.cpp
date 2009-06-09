#include "MultiThreadedSurfaceExtractor.h"

#include "SurfaceExtractorThread.h"

using namespace PolyVox;

MultiThreadedSurfaceExtractor::MultiThreadedSurfaceExtractor(Volume<PolyVox::uint8_t>* pVolData, unsigned int noOfThreads)
:m_pVolData(pVolData)
,m_bFinished(false)
{
	//m_pSurfaceExtractorThread = new SurfaceExtractorThread(this);
	//m_pSurfaceExtractorThread2 = new SurfaceExtractorThread(this);

	m_mutexPendingTasks = new QMutex();
	m_mutexCompletedTasks = new QMutex();

	m_noOfTasksAvailable = new QSemaphore();

	m_vecThreads.resize(noOfThreads);
	for(int ct = 0; ct < noOfThreads; ++ct)
	{
		m_vecThreads[ct] = new SurfaceExtractorThread(this);
		m_vecThreads[ct]->start();
	}
}
	
void MultiThreadedSurfaceExtractor::addTask(Region regToProcess, uint8_t uLodLevel)
{
	TaskData taskData;
	taskData.m_uLodLevel = uLodLevel;
	taskData.m_regToProcess = regToProcess;

	m_mutexPendingTasks->lock();
	m_queuePendingTasks.push(taskData);
	m_mutexPendingTasks->unlock();

	m_noOfTasksAvailable->release();
}
