#include "LoadingProgress.h"

#include "ThermiteGameLogic.h"

namespace Thermite
{
	LoadingProgress::LoadingProgress(QWidget* parent, Qt::WindowFlags f)
		:QWidget(parent, f)
	{
		setupUi(this);

		m_progLoadingData->setRange(0,100);
		m_progExtractingSurface->setRange(0,100);

		m_progLoadingData->setValue(0);
		m_progExtractingSurface->setValue(0);
	}

	void LoadingProgress::setLoadingDataPercentageDone(unsigned int percentageDone)
	{
		m_progLoadingData->setValue(percentageDone);
	}

	void LoadingProgress::setExtractingSurfacePercentageDone(unsigned int percentageDone)
	{
		m_progExtractingSurface->setValue(percentageDone);
	}
}
