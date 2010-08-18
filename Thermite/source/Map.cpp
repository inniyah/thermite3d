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

#include "Map.h"

#include "Application.h"
#include "MapHandler.h"
#include "VolumeManager.h"

#include "VolumeChangeTracker.h"

#include "MapRegion.h"

#include "ThermiteGameLogic.h"

#include "MaterialDensityPair.h"

#ifdef ENABLE_BULLET_PHYSICS
	#include "OgreBulletDynamicsWorld.h"
#endif //ENABLE_BULLET_PHYSICS

#include <OgreSceneManagerEnumerator.h>
#include <OgreSceneManager.h>

#include <QSettings>

//using namespace Ogre;
using namespace PolyVox;

namespace Thermite
{
	Map::Map(QObject* parent)
		:Object(parent)
	{
		m_pOgreSceneManager = 0;
	}

	Map::~Map(void)
	{

	}

	void Map::initialise(void)
	{
		int regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();

		volumeChangeTracker = new VolumeChangeTracker<MaterialDensityPair44>(volumeResource->getVolume(), regionSideLength);
		volumeChangeTracker->setAllRegionsModified();

		int volumeWidthInRegions = volumeChangeTracker->getWrappedVolume()->getWidth() / regionSideLength;
		int volumeHeightInRegions = volumeChangeTracker->getWrappedVolume()->getHeight() / regionSideLength;
		int volumeDepthInRegions = volumeChangeTracker->getWrappedVolume()->getDepth() / regionSideLength;

		m_volRegionTimeStamps = new Volume<std::uint32_t>(volumeWidthInRegions, volumeHeightInRegions, volumeDepthInRegions, 0);
		m_volMapRegions = new Volume<MapRegion*>(volumeWidthInRegions, volumeHeightInRegions, volumeDepthInRegions, 0);
		m_volRegionBeingProcessed = new Volume<bool>(volumeWidthInRegions, volumeHeightInRegions, volumeDepthInRegions, 0);
		m_volSurfaceDecimators = new Volume<SurfaceMeshDecimationTask*>(volumeWidthInRegions, volumeHeightInRegions, volumeDepthInRegions, 0);
	}

	void  Map::createSphereAt(QVector3D centre, float radius, int value, bool bPaintMode)
	{
		int firstX = static_cast<int>(std::floor(centre.x() - radius));
		int firstY = static_cast<int>(std::floor(centre.y() - radius));
		int firstZ = static_cast<int>(std::floor(centre.z() - radius));

		int lastX = static_cast<int>(std::ceil(centre.x() + radius));
		int lastY = static_cast<int>(std::ceil(centre.y() + radius));
		int lastZ = static_cast<int>(std::ceil(centre.z() + radius));

		float radiusSquared = radius * radius;

		//Check bounds
		firstX = std::max(firstX,0);
		firstY = std::max(firstY,0);
		firstZ = std::max(firstZ,0);

		lastX = std::min(lastX,int(volumeChangeTracker->getWrappedVolume()->getWidth()-1));
		lastY = std::min(lastY,int(volumeChangeTracker->getWrappedVolume()->getHeight()-1));
		lastZ = std::min(lastZ,int(volumeChangeTracker->getWrappedVolume()->getDepth()-1));

		PolyVox::Region regionToLock = PolyVox::Region(PolyVox::Vector3DInt16(firstX, firstY, firstZ), PolyVox::Vector3DInt16(lastX, lastY, lastZ));

		////////////////////////////////////////////////////////////////////////////////

		//This is ugly, but basically we are making sure that we do not modify part of the volume of the mesh is currently
		//being regenerated for that part. This is to avoid 'queing up' a whole bunch of surface exreaction commands for 
		//the same region, only to have them rejected because the time stamp has changed again since they were issued.

		//At this point it probably makes sense to pull the VolumeChangeTracker from PolyVox into Thermite and have it
		//handle these checks as well.

		//Longer term, it might be interesting to introduce a 'ModyfyVolumeCommand' which can be issued to runn on seperate threads.
		//We could then schedule these so that all the ones for a given region are processed before we issue the extract surface command
		//for that region.
		const std::uint16_t firstRegionX = regionToLock.getLowerCorner().getX() >> volumeChangeTracker->m_uRegionSideLengthPower;
		const std::uint16_t firstRegionY = regionToLock.getLowerCorner().getY() >> volumeChangeTracker->m_uRegionSideLengthPower;
		const std::uint16_t firstRegionZ = regionToLock.getLowerCorner().getZ() >> volumeChangeTracker->m_uRegionSideLengthPower;

		const std::uint16_t lastRegionX = regionToLock.getUpperCorner().getX() >> volumeChangeTracker->m_uRegionSideLengthPower;
		const std::uint16_t lastRegionY = regionToLock.getUpperCorner().getY() >> volumeChangeTracker->m_uRegionSideLengthPower;
		const std::uint16_t lastRegionZ = regionToLock.getUpperCorner().getZ() >> volumeChangeTracker->m_uRegionSideLengthPower;

		for(std::uint16_t zCt = firstRegionZ; zCt <= lastRegionZ; zCt++)
		{
			for(std::uint16_t yCt = firstRegionY; yCt <= lastRegionY; yCt++)
			{
				for(std::uint16_t xCt = firstRegionX; xCt <= lastRegionX; xCt++)
				{
					//volRegionLastModified->setVoxelAt(xCt,yCt,zCt,m_uCurrentTime);
					if(m_volRegionBeingProcessed->getVoxelAt(xCt,yCt,zCt))
					{
						return;
					}
				}
			}
		}
		////////////////////////////////////////////////////////////////////////////////

		volumeChangeTracker->lockRegion(regionToLock);
		for(int z = firstZ; z <= lastZ; ++z)
		{
			for(int y = firstY; y <= lastY; ++y)
			{
				for(int x = firstX; x <= lastX; ++x)
				{
					if((centre - QVector3D(x,y,z)).lengthSquared() <= radiusSquared)
					{
						MaterialDensityPair44 currentValue = volumeChangeTracker->getWrappedVolume()->getVoxelAt(x,y,z);
						currentValue.setDensity(MaterialDensityPair44::getMaxDensity());
						volumeChangeTracker->setLockedVoxelAt(x,y,z,currentValue);
					}
				}
			}
		}
		volumeChangeTracker->unlockRegion();
	}

	QVector3D Map::getRayVolumeIntersection(QVector3D rayOrigin, const QVector3D& rayDir)
	{
		//Initialise to failure
		/*std::pair<bool, QVector3D> result;
		result.first = false;
		result.second = QVector3D(0,0,0);*/

		QVector3D result = QVector3D(0,0,0);

		//Ensure the voume is valid
		PolyVox::Volume<MaterialDensityPair44>* pVolume = volumeResource->getVolume();
		if(pVolume == 0)
		{
			return result;
		}

		Ogre::Real dist = 0.0f;
		for(int steps = 0; steps < 1000; steps++)
		{
			//Ogre::Vector3 point = ray.getPoint(dist);
			//PolyVox::Vector3DUint16 v3dPoint = PolyVox::Vector3DUint16(point.x + 0.5, point.y + 0.5, point.z + 0.5);
			rayOrigin += rayDir.normalized();

			if(pVolume->getVoxelAt(rayOrigin.x(), rayOrigin.y(), rayOrigin.z()).getMaterial() > 0)
			{
				result = rayOrigin;
				return result;
			}

			dist += 1.0f;			
		}

		return result;
	}
}