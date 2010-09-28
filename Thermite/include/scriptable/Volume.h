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

#include "Object.h"
#include "QtForwardDeclarations.h"
#include "ThermiteForwardDeclarations.h"

#include "Array.h"
#include "PolyVoxForwardDeclarations.h"

#include <QScriptEngine>

#include <map>
#include <set>

namespace Thermite
{
	class Volume : public Object
	{
		Q_OBJECT

	public:
		Volume(QObject* parent = 0);
		~Volume(void);

		void setPolyVoxVolume(PolyVox::Volume<PolyVox::MaterialDensityPair44>* pPolyVoxVolume, uint16_t regionSideLength);

		void initialise(void);
		void updatePolyVoxGeometry(const QVector3D& cameraPos);

	public slots:
		void createCuboidAt(QVector3D centre, QVector3D dimensions, int material, int density, bool bPaintMode);
		void createSphereAt(QVector3D centre, float radius, int value, bool bPaintMode);
		QVector3D getRayVolumeIntersection(QVector3D rayOrigin, const QVector3D& rayDir);

		bool loadFromFile(const QString& filename);

		void uploadSurfaceExtractorResult(SurfaceMeshExtractionTask* pTask);
		void uploadSurfaceDecimatorResult(SurfaceMeshDecimationTask* pTask);

		void generateMapForTankWars(void);

	public:
		static TaskProcessorThread* m_backgroundThread;

		PolyVox::Volume<PolyVox::MaterialDensityPair44>* m_pPolyVoxVolume;
		uint16_t mRegionSideLength;
		uint16_t mVolumeWidthInRegions;
		uint16_t mVolumeHeightInRegions;
		uint16_t mVolumeDepthInRegions; 

		std::map< std::string, std::set<uint8_t> > m_mapMaterialIds;	

		
		PolyVox::Array<3, PolyVox::SurfaceMesh*> m_volSurfaceMeshes;
		PolyVox::Array<3, uint32_t> mLastModifiedArray;
		PolyVox::Array<3, uint32_t> mExtractionStartedArray;
		PolyVox::Array<3, uint32_t> mExtractionFinishedArray;
		PolyVox::Array<3, bool> mRegionBeingExtracted;
		PolyVox::Array<3, SurfaceMeshDecimationTask*> m_volSurfaceDecimators;

	private:
		bool isRegionBeingExtracted(const PolyVox::Region& regionToTest);
		void updateLastModifedArray(const PolyVox::Region& regionToTest);
	};	
}

Q_SCRIPT_DECLARE_QMETAOBJECT(Thermite::Volume, QObject*)


#endif //__THERMITE_VOLUME_H__