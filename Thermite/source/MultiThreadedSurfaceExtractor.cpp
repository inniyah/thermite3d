#include "MultiThreadedSurfaceExtractor.h"

#include "SurfaceExtractorThread.h"

#include <utility>

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
	m_noOfResultsAvailable = new QSemaphore();

	m_vecThreads.resize(noOfThreads);
	for(int ct = 0; ct < noOfThreads; ++ct)
	{
		m_vecThreads[ct] = new SurfaceExtractorThread(this);
	}
}
	
void MultiThreadedSurfaceExtractor::addTask(const SurfaceExtractorTaskData& taskData)
{
	m_mutexPendingTasks->lock();
	m_queuePendingTasks.push(taskData);
	m_mutexPendingTasks->unlock();

	m_noOfTasksAvailable->release();
}

int MultiThreadedSurfaceExtractor::noOfResultsAvailable(void)
{
	return m_noOfResultsAvailable->available();
}

SurfaceExtractorTaskData MultiThreadedSurfaceExtractor::getResult(void)
{
	SurfaceExtractorTaskData result;

	if(m_noOfResultsAvailable->tryAcquire())
	{
		m_mutexCompletedTasks->lock();
		result = m_listCompletedTasks.front();
		m_listCompletedTasks.pop_front();
		m_mutexCompletedTasks->unlock();
	}
	else
	{
		//this shouldn't really happen, and the user should have called isResultAvailable() first.
		//Maybe we should throw an exception here instead?
		Vector3DInt16 v3dZero(0,0,0);
		Region regZero(v3dZero, v3dZero);
		result.setLodLevel(0);
		result.setRegion(regZero);
	}

	return result;
}

void MultiThreadedSurfaceExtractor::start(void)
{
	for(int ct = 0; ct < m_vecThreads.size(); ++ct)
	{
		m_vecThreads[ct]->start();
	}
}