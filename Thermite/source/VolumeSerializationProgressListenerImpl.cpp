#include "VolumeSerializationProgressListenerImpl.h"

#include "ThermiteGameLogic.h"

namespace Thermite
{
	VolumeSerializationProgressListenerImpl::VolumeSerializationProgressListenerImpl(ThermiteGameLogic* pThermiteGameLogic)
		:m_pThermiteGameLogic(pThermiteGameLogic)
	{
	}

	void VolumeSerializationProgressListenerImpl::onProgressUpdated(float fProgress)
	{
		m_pThermiteGameLogic->setVolumeLoadProgress(fProgress);
	}
}