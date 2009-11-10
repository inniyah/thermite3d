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

#include "SurfaceExtractorThread.h"

#include "MultiThreadedSurfaceExtractor.h"
#include "SurfaceExtractorTaskData.h"

#include "GradientEstimators.h"
#include "IndexedSurfacePatch.h"

using namespace PolyVox;

namespace Thermite
{
	SurfaceExtractorThread::SurfaceExtractorThread(MultiThreadedSurfaceExtractor* pParentMTSE, PolyVox::Volume<PolyVox::uint8_t>* pVolData)
	:m_pParentMTSE(pParentMTSE)
	,m_pVolData(pVolData)
	{
		m_pSurfaceExtractor = new SurfaceExtractor(*(pVolData));
	}

	SurfaceExtractorThread::~SurfaceExtractorThread()
	{
		delete m_pSurfaceExtractor;
	}

	void SurfaceExtractorThread::run(void)
	{
		while(true)
		{
			SurfaceExtractorTaskData taskData = m_pParentMTSE->popTask();
			m_pSurfaceExtractor->setLodLevel(taskData.m_uLodLevel);
			taskData.m_ispResult = m_pSurfaceExtractor->extractSurfaceForRegion(taskData.m_regToProcess);

			//taskData.m_ispResult->generateAveragedFaceNormals(true);
			computeNormalsForVertices(m_pVolData, *(taskData.m_ispResult.get()), SOBEL);
			taskData.m_ispResult->smoothPositions(0.1f);
			taskData.m_ispResult->smoothPositions(0.1f);
			taskData.m_ispResult->smoothPositions(0.1f);

			m_pParentMTSE->pushResult(taskData);
		}
	}
}
