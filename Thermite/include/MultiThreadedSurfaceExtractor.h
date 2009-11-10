#pragma region License
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
#pragma endregion

#ifndef __THERMITE_MULTITHREADEDSURFACEEXTRACTOR_H__
#define __THERMITE_MULTITHREADEDSURFACEEXTRACTOR_H__

#include <list>
#include <queue>

#include "SurfaceExtractorTaskData.h"

class QMutex;
class QSemaphore;

namespace Thermite
{
	class SurfaceExtractorThread;

	class SurfaceExtractorTaskDataPriorityComparison
	{
	public:
	  bool operator() (const SurfaceExtractorTaskData& lhs, const SurfaceExtractorTaskData &rhs) const
	  {
		  return lhs.getPriority() < rhs.getPriority();
	  }
	};


	class MultiThreadedSurfaceExtractor
	{
	public:
		MultiThreadedSurfaceExtractor(PolyVox::Volume<PolyVox::uint8_t>* pVolData, unsigned int noOfThreads);
		~MultiThreadedSurfaceExtractor();
		
		void pushTask(const SurfaceExtractorTaskData& taskData);
		SurfaceExtractorTaskData popTask(void);

		void pushResult(const SurfaceExtractorTaskData& taskData);
		SurfaceExtractorTaskData popResult(void);

		int noOfResultsAvailable(void);

		void start(void);

	private:
		PolyVox::Volume<PolyVox::uint8_t>* m_pVolData;

		std::priority_queue<SurfaceExtractorTaskData, std::vector<SurfaceExtractorTaskData>, SurfaceExtractorTaskDataPriorityComparison> m_queuePendingTasks;
		std::list<SurfaceExtractorTaskData> m_listCompletedTasks;

		QMutex* m_mutexPendingTasks;
		QMutex* m_mutexCompletedTasks;

		QSemaphore* m_noOfTasksAvailable;
		QSemaphore* m_noOfResultsAvailable;

		std::vector<SurfaceExtractorThread*> m_vecThreads;
	};
}

#endif
