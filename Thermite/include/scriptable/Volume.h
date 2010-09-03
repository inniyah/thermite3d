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

#ifndef __THERMITE_VOLUME_H__
#define __THERMITE_VOLUME_H__

#include "ThermiteForwardDeclarations.h"
#include "VolumeChangeTracker.h"

#include "SurfaceExtractorTaskData.h"

#include "PolyVoxForwardDeclarations.h"
#include "Array.h"

#include <map>

#include "Object.h"

#include <QScriptEngine>
#include <QVector3D>

namespace Thermite
{
	class Volume : public Object
	{
		Q_OBJECT

	public:
		Volume(QObject* parent = 0);
		~Volume(void);

		void initialise(void);

		void updatePolyVoxGeometry(const QVector3D& cameraPos);

	public slots:
		void createSphereAt(QVector3D centre, float radius, int value, bool bPaintMode);
		QVector3D getRayVolumeIntersection(QVector3D rayOrigin, const QVector3D& rayDir);

		bool loadFromFile(const QString& filename);

		void surfaceExtractionFinished(SurfaceExtractorTaskData result);

		void uploadSurfaceExtractorResult(SurfaceExtractorTaskData result);
		void uploadSurfaceDecimatorResult(SurfaceExtractorTaskData result);

	public:

		static TaskProcessorThread* m_backgroundThread;

		polyvox_shared_ptr< PolyVox::Volume<PolyVox::MaterialDensityPair44> > m_pPolyVoxVolume;

		std::map< std::string, std::set<uint8_t> > m_mapMaterialIds;	

		PolyVox::VolumeChangeTracker<PolyVox::MaterialDensityPair44>* volumeChangeTracker;

		
		PolyVox::Array<3, PolyVox::SurfaceMesh*> m_volSurfaceMeshes;
		PolyVox::Array<3, uint32_t> m_volRegionTimeStamps;
		PolyVox::Array<3, uint32_t> m_volLatestMeshTimeStamps;
		PolyVox::Array<3, bool> m_volRegionBeingProcessed;
		PolyVox::Array<3, SurfaceMeshDecimationTask*> m_volSurfaceDecimators;
	};	
}

Q_SCRIPT_DECLARE_QMETAOBJECT(Thermite::Volume, QObject*)


#endif //__THERMITE_VOLUME_H__