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
