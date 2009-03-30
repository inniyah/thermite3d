#include "TimeStampedSurfacePatchCache.h"
#include "ThermiteForwardDeclarations.h"

#include "PolyVoxCore/Volume.h"
#include "PolyVoxCore/GradientEstimators.h"
#include "PolyVoxCore/SurfaceExtractors.h"

using namespace PolyVox;
using namespace std;

TimeStampedSurfacePatchCache* TimeStampedSurfacePatchCache::m_pInstance = 0;

TimeStampedSurfacePatchCache::TimeStampedSurfacePatchCache()
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

	//Get the time stamps
	int32_t regionTimeStamp = m_vctTracker->getLastModifiedTimeForRegion(position.getX()/THERMITE_REGION_SIDE_LENGTH,position.getY()/THERMITE_REGION_SIDE_LENGTH,position.getZ()/THERMITE_REGION_SIDE_LENGTH);
	int32_t ispTimeStamp = ispResult->m_iTimeStamp;

	if(regionTimeStamp > ispTimeStamp) //Need to regenerate mesh
	{
		const uint16_t firstX = position.getX();
		const uint16_t firstY = position.getY();
		const uint16_t firstZ = position.getZ();
		const uint16_t lastX = firstX + THERMITE_REGION_SIDE_LENGTH;
		const uint16_t lastY = firstY + THERMITE_REGION_SIDE_LENGTH;
		const uint16_t lastZ = firstZ + THERMITE_REGION_SIDE_LENGTH;

		Region region(Vector3DInt32(firstX, firstY, firstZ), Vector3DInt32(lastX, lastY, lastZ));
		region.cropTo(m_vctTracker->getVolumeData()->getEnclosingRegion());

		extractSurface(m_vctTracker->getVolumeData(), lod, region, ispResult);
		computeNormalsForVertices(m_vctTracker->getVolumeData(), *ispResult, CENTRAL_DIFFERENCE_SMOOTHED);
		ispResult->m_iTimeStamp = m_vctTracker->getCurrentTime();
	}

	return ispResult;
}