#pragma region License
/******************************************************************************
This file is part of the Thermite 3D game engine
Copyright (C) 2006  David Williams

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
******************************************************************************/
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
