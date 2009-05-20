#include "TimeStampedSurfacePatchCache.h"
#include "ThermiteForwardDeclarations.h"

#include "Volume.h"
#include "GradientEstimators.h"
#include "SurfaceAdjusters.h"
#include "SurfaceExtractor.h"
#include "SurfaceExtractors.h"

#include "Application.h"

#include <QSettings>

using namespace PolyVox;
using namespace std;

TimeStampedSurfacePatchCache* TimeStampedSurfacePatchCache::m_pInstance = 0;

TimeStampedSurfacePatchCache::TimeStampedSurfacePatchCache()
	:m_pSurfaceExtractor(0)
{
}

TimeStampedSurfacePatchCache* TimeStampedSurfacePatchCache::getInstance()
{
	if(m_pInstance == 0)
	{
		m_pInstance = new TimeStampedSurfacePatchCache();
	}

	return m_pInstance;
}

IndexedSurfacePatch* TimeStampedSurfacePatchCache::getIndexedSurfacePatch(Vector3DInt32 position, PolyVox::uint8_t lod)
{
	//Get the surface patch, creating it if it doesn't exist.
	std::map<Vector3DInt32, IndexedSurfacePatch*>& mapIsp = m_mapIsps[lod];

	IndexedSurfacePatch* ispResult;
	std::map<Vector3DInt32, IndexedSurfacePatch*>::iterator iterMapIsp = mapIsp.find(position);
	if(iterMapIsp == mapIsp.end())
	{
		ispResult = new IndexedSurfacePatch();
		mapIsp.insert(make_pair(position, ispResult));
	}
	else
	{
		ispResult = iterMapIsp->second;
	}

	if(m_pSurfaceExtractor == 0) //This is dodgy - think about lifetime issues.
	{
		m_pSurfaceExtractor = new SurfaceExtractor(*(m_vctTracker->getWrappedVolume()));
	}

	//Get the time stamps
	int regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();
	int32_t regionTimeStamp = m_vctTracker->getLastModifiedTimeForRegion(position.getX()/regionSideLength,position.getY()/regionSideLength,position.getZ()/regionSideLength);
	int32_t ispTimeStamp = ispResult->m_iTimeStamp;

	if(regionTimeStamp > ispTimeStamp) //Need to regenerate mesh
	{
		const uint16_t firstX = position.getX();
		const uint16_t firstY = position.getY();
		const uint16_t firstZ = position.getZ();
		const uint16_t lastX = firstX + regionSideLength;
		const uint16_t lastY = firstY + regionSideLength;
		const uint16_t lastZ = firstZ + regionSideLength;

		Region region(Vector3DInt32(firstX, firstY, firstZ), Vector3DInt32(lastX, lastY, lastZ));
		region.cropTo(m_vctTracker->getWrappedVolume()->getEnclosingRegion());

		//extractSurface(m_vctTracker->getWrappedVolume(), lod, region, ispResult);
		m_pSurfaceExtractor->setLodLevel(lod);
		m_pSurfaceExtractor->extractSurfaceForRegion(region, ispResult);
		computeNormalsForVertices(m_vctTracker->getWrappedVolume(), *ispResult, CENTRAL_DIFFERENCE_SMOOTHED);
		//*ispResult = getSmoothedSurface(*ispResult);
		ispResult->m_iTimeStamp = m_vctTracker->getCurrentTime();
	}

	return ispResult;
}