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

#include "Region.h"
#include "IndexedSurfacePatch.h"

#include <list>
#include <queue>

class SurfaceExtractorThread;

class TaskData
{
public:
	PolyVox::uint8_t m_uLodLevel;
	PolyVox::Region m_regToProcess;
	POLYVOX_SHARED_PTR<PolyVox::IndexedSurfacePatch> m_ispResult;
};

class MultiThreadedSurfaceExtractor
{
public:
	MultiThreadedSurfaceExtractor(PolyVox::Volume<PolyVox::uint8_t>* pVolData);
	
	void addTask(PolyVox::Region regToProcess, PolyVox::uint8_t uLodLevel);

	PolyVox::Volume<PolyVox::uint8_t>* m_pVolData;
	std::queue<TaskData> m_queuePendingTasks;
	std::list<TaskData> m_listCompletedTasks;

	SurfaceExtractorThread* m_pSurfaceExtractorThread;

	bool m_bFinished;
};

#endif
