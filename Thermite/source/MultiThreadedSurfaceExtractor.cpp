/*******************************************************************************
Copyright (c) 2005-2009 David Williams

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution. 	
*******************************************************************************/

#include "MultiThreadedSurfaceExtractor.h"

#include "SurfaceExtractorThread.h"

#include <QMutex>
#include <QSemaphore>

#include <utility>

using namespace PolyVox;

namespace Thermite
{
	MultiThreadedSurfaceExtractor::MultiThreadedSurfaceExtractor(Volume<PolyVox::uint8_t>* pVolData, unsigned int noOfThreads)
	:m_pVolData(pVolData)
	{
		m_mutexPendingTasks = new QMutex();
		m_mutexCompletedTasks = new QMutex();

		m_noOfTasksAvailable = new QSemaphore();
		m_noOfResultsAvailable = new QSemaphore();

		m_vecThreads.resize(noOfThreads);
		for(int ct = 0; ct < noOfThreads; ++ct)
		{
			m_vecThreads[ct] = new SurfaceExtractorThread(this, pVolData);
		}
	}

	MultiThreadedSurfaceExtractor::~MultiThreadedSurfaceExtractor()
	{
		for(int ct = 0; ct < m_vecThreads.size(); ++ct)
		{
			m_vecThreads[ct]->quit();
			delete m_vecThreads[ct];
		}

		delete m_mutexPendingTasks;
		delete m_mutexCompletedTasks;

		delete m_noOfTasksAvailable;
		delete m_noOfResultsAvailable;
	}
		
	void MultiThreadedSurfaceExtractor::pushTask(const SurfaceExtractorTaskData& taskData)
	{
		//Add the task to the queue
		m_mutexPendingTasks->lock();
		m_queuePendingTasks.push(taskData);
		m_mutexPendingTasks->unlock();

		//Increment the number of tasks available
		m_noOfTasksAvailable->release();
	}

	SurfaceExtractorTaskData MultiThreadedSurfaceExtractor::popTask(void)
	{
		//Decrement the number of tasks available
		m_noOfTasksAvailable->acquire();

		//Remove the test from the queue
		m_mutexPendingTasks->lock();
		SurfaceExtractorTaskData taskData = m_queuePendingTasks.top();
		m_queuePendingTasks.pop();
		m_mutexPendingTasks->unlock();

		//Return the task
		return taskData;
	}

	int MultiThreadedSurfaceExtractor::noOfResultsAvailable(void)
	{
		return m_noOfResultsAvailable->available();
	}

	void MultiThreadedSurfaceExtractor::pushResult(const SurfaceExtractorTaskData& taskData)
	{
		//Add the result to the list
		m_mutexCompletedTasks->lock();
		m_listCompletedTasks.push_back(taskData);
		m_mutexCompletedTasks->unlock();

		//Increment the number of results available
		m_noOfResultsAvailable->release();
	}

	SurfaceExtractorTaskData MultiThreadedSurfaceExtractor::popResult(void)
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
}