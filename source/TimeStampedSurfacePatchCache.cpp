#include "TimeStampedSurfacePatchCache.h"

#include "PolyVoxCore/BlockVolume.h"
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

IndexedSurfacePatch* TimeStampedSurfacePatchCache::getIndexedSurfacePatch(Vector3DInt32 position, PolyVox::uint8 lod)
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
	int32 regionTimeStamp = m_vctTracker->getLastModifiedTimeForRegion(position.getX()/POLYVOX_REGION_SIDE_LENGTH,position.getY()/POLYVOX_REGION_SIDE_LENGTH,position.getZ()/POLYVOX_REGION_SIDE_LENGTH);
	int32 ispTimeStamp = ispResult->m_iTimeStamp;

	if(regionTimeStamp > ispTimeStamp) //Need to regenerate mesh
	{
		const uint16 firstX = position.getX();
		const uint16 firstY = position.getY();
		const uint16 firstZ = position.getZ();
		const uint16 lastX = firstX + POLYVOX_REGION_SIDE_LENGTH;
		const uint16 lastY = firstY + POLYVOX_REGION_SIDE_LENGTH;
		const uint16 lastZ = firstZ + POLYVOX_REGION_SIDE_LENGTH;

		Region region(Vector3DInt32(firstX, firstY, firstZ), Vector3DInt32(lastX, lastY, lastZ));
		region.cropTo(m_vctTracker->getVolumeData()->getEnclosingRegion());

		extractSurface(m_vctTracker->getVolumeData(), lod, region, ispResult);

		ispResult->m_v3dRegionPosition = position;
		ispResult->m_iTimeStamp = m_vctTracker->getCurrentTime();
	}

	return ispResult;
}