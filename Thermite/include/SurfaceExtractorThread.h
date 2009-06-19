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

#ifndef __THERMITE_SURFACEEXTRACTORTHREAD_H__
#define __THERMITE_SURFACEEXTRACTORTHREAD_H__

#include "SurfaceExtractor.h"

#include <QThread>

namespace Thermite
{
	class MultiThreadedSurfaceExtractor;

	class SurfaceExtractorThread : public QThread
	{
		Q_OBJECT

	public:
		SurfaceExtractorThread(MultiThreadedSurfaceExtractor* pParentMTSE, PolyVox::Volume<PolyVox::uint8_t>* pVolData);
		~SurfaceExtractorThread();

	protected:
		void run(void);

	private:
		PolyVox::Volume<PolyVox::uint8_t>* m_pVolData;
		PolyVox::SurfaceExtractor* m_pSurfaceExtractor;
		MultiThreadedSurfaceExtractor* m_pParentMTSE;
	};
}

#endif
