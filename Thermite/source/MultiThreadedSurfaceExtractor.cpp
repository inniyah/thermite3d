#include "MultiThreadedSurfaceExtractor.h"

#include "SurfaceExtractorThread.h"

using namespace PolyVox;

MultiThreadedSurfaceExtractor::MultiThreadedSurfaceExtractor(Volume<PolyVox::uint8_t>* pVolData)
:m_pVolData(pVolData)
,m_bFinished(false)
{
	m_pSurfaceExtractorThread = new SurfaceExtractorThread(this);
	m_pSurfaceExtractorThread2 = new SurfaceExtractorThread(this);

	m_mutexPendingTasks = new QMutex();
	m_mutexCompletedTasks = new QMutex();
}
	
void MultiThreadedSurfaceExtractor::addTask(Region regToProcess, uint8_t uLodLevel)
{
	TaskData taskData;
	taskData.m_uLodLevel = uLodLevel;
	taskData.m_regToProcess = regToProcess;

	m_mutexPendingTasks->lock();
	m_queuePendingTasks.push(taskData);
	m_mutexPendingTasks->unlock();
}
