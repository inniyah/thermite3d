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

#include "AmbientOcclusionTask.h"

#include "PolyVoxCore/Material.h"

#include "PolyVoxCore/AmbientOcclusionCalculator.h"

#include <QMutex>

using namespace PolyVox;

namespace Thermite
{
	AmbientOcclusionTask::AmbientOcclusionTask(PolyVox::SimpleVolume<PolyVox::Material16>* volume, PolyVox::Array<3, uint8_t>* ambientOcclusionVolume, PolyVox::Region regToProcess, uint32_t uTimeStamp, float rayLength)
		:m_regToProcess(regToProcess)
		,mAmbientOcclusionVolume(ambientOcclusionVolume)
		,m_uTimeStamp(uTimeStamp)
		,mVolume(volume)
		,mRayLength(rayLength)
	{
	}
	
	void AmbientOcclusionTask::run(void)
	{	
		uint8_t uNoOfSamplesPerOutputElement = 0; //Max off 255 for max quality.
		AmbientOcclusionCalculator<SimpleVolume, Material16> ambientOcclusionCalculator(mVolume, mAmbientOcclusionVolume, m_regToProcess, mRayLength, uNoOfSamplesPerOutputElement);
		ambientOcclusionCalculator.execute();

		emit finished(this);
	}
}
