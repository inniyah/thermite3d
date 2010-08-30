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

#include "SurfaceMeshExtractionTask.h"
#include "SurfaceMeshDecimationTask.h"

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

		m_backgroundThread = new TaskProcessorThread;
		m_backgroundThread->setPriority(QThread::LowestPriority);
		m_backgroundThread->start();
	}

	void Map::updatePolyVoxGeometry(Ogre::Vector3 cameraPos)
	{
		/*if(mMap == 0)
			return;*/

		if(!volumeResource.isNull())
		{		
			//Some values we'll need later.
			std::uint16_t regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();
			std::uint16_t halfRegionSideLength = regionSideLength / 2;
			std::uint16_t volumeWidthInRegions = volumeResource->getVolume()->getWidth() / regionSideLength;
			std::uint16_t volumeHeightInRegions = volumeResource->getVolume()->getHeight() / regionSideLength;
			std::uint16_t volumeDepthInRegions = volumeResource->getVolume()->getDepth() / regionSideLength;

			//Iterate over each region in the VolumeChangeTracker
			for(std::uint16_t regionZ = 0; regionZ < volumeDepthInRegions; ++regionZ)
			{		
				for(std::uint16_t regionY = 0; regionY < volumeHeightInRegions; ++regionY)
				{
					for(std::uint16_t regionX = 0; regionX < volumeWidthInRegions; ++regionX)
					{
						//Compute the extents of the current region
						const std::uint16_t firstX = regionX * regionSideLength;
						const std::uint16_t firstY = regionY * regionSideLength;
						const std::uint16_t firstZ = regionZ * regionSideLength;

						const std::uint16_t lastX = firstX + regionSideLength;
						const std::uint16_t lastY = firstY + regionSideLength;
						const std::uint16_t lastZ = firstZ + regionSideLength;	

						const float centreX = firstX + halfRegionSideLength;
						const float centreY = firstY + halfRegionSideLength;
						const float centreZ = firstZ + halfRegionSideLength;

						//The regions distance from the camera is used for prioritizing surface extraction
						Ogre::Vector3 centre(centreX, centreY, centreZ);
						double distanceFromCameraSquared = (cameraPos - centre).squaredLength();

						//There's no guarentee that the MapRegion exists at this point...
						MapRegion* pMapRegion = m_volMapRegions->getVoxelAt(regionX, regionY, regionZ);

						//If the region has changed then we may need to add or remove MapRegion to/from the scene graph
						std::uint32_t uRegionTimeStamp = volumeChangeTracker->getLastModifiedTimeForRegion(regionX, regionY, regionZ);
						if(uRegionTimeStamp > m_volRegionTimeStamps->getVoxelAt(regionX,regionY,regionZ))
						{
							m_volRegionBeingProcessed->setVoxelAt(regionX,regionY,regionZ,true);

							//Convert to a real PolyVox::Region
							Vector3DInt16 v3dLowerCorner(firstX,firstY,firstZ);
							Vector3DInt16 v3dUpperCorner(lastX,lastY,lastZ);
							PolyVox::Region region(v3dLowerCorner, v3dUpperCorner);
							region.cropTo(volumeChangeTracker->getWrappedVolume()->getEnclosingRegion());

							//The prioirty ensures that the surfaces for regions close to the
							//camera get extracted before those which are distant from the camera.
							std::uint32_t uPriority = std::numeric_limits<std::uint32_t>::max() - static_cast<std::uint32_t>(distanceFromCameraSquared);

							//Extract the region
							SurfaceExtractorTaskData taskData(volumeResource->getVolume(), region, uRegionTimeStamp);
							SurfaceMeshExtractionTask* surfaceMeshExtractionTask = new SurfaceMeshExtractionTask(taskData);
							QObject::connect(surfaceMeshExtractionTask, SIGNAL(finished(SurfaceExtractorTaskData)), this, SLOT(uploadSurfaceExtractorResult(SurfaceExtractorTaskData)), Qt::QueuedConnection);
							QThreadPool::globalInstance()->start(surfaceMeshExtractionTask, uPriority);

							//Indicate that we've processed this region
							m_volRegionTimeStamps->setVoxelAt(regionX,regionY,regionZ,volumeChangeTracker->getLastModifiedTimeForRegion(regionX, regionY, regionZ));
						}
					}
				}
			}
		}
	}

	void Map::uploadSurfaceExtractorResult(SurfaceExtractorTaskData result)
	{
		std::uint16_t regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();

		//Determine where it came from
		uint16_t regionX = result.getRegion().getLowerCorner().getX() / regionSideLength;
		uint16_t regionY = result.getRegion().getLowerCorner().getY() / regionSideLength;
		uint16_t regionZ = result.getRegion().getLowerCorner().getZ() / regionSideLength;

		std::uint32_t uRegionTimeStamp = volumeChangeTracker->getLastModifiedTimeForRegion(regionX, regionY, regionZ);
		if(uRegionTimeStamp > result.m_uTimeStamp)
		{
			// The volume has changed since the command to generate this mesh was issued.
			// Just ignore it, and a correct version should be along soon...
			return;
		}

		uploadSurfaceMesh(result.getSurfaceMesh(), result.getRegion());


		SurfaceMeshDecimationTask* pOldSurfaceDecimator = m_volSurfaceDecimators->getVoxelAt(regionX, regionY, regionZ);

		m_backgroundThread->removeTask(pOldSurfaceDecimator);

		SurfaceMeshDecimationTask* surfaceMeshDecimationTask = new SurfaceMeshDecimationTask(result);
		QObject::connect(surfaceMeshDecimationTask, SIGNAL(finished(SurfaceExtractorTaskData)), this, SLOT(uploadSurfaceDecimatorResult(SurfaceExtractorTaskData)), Qt::QueuedConnection);

		m_volSurfaceDecimators->setVoxelAt(regionX, regionY, regionZ, surfaceMeshDecimationTask);

		m_backgroundThread->addTask(surfaceMeshDecimationTask);
	}

	void Map::uploadSurfaceDecimatorResult(SurfaceExtractorTaskData result)
	{
		std::uint16_t regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();

		//Determine where it came from
		uint16_t regionX = result.getRegion().getLowerCorner().getX() / regionSideLength;
		uint16_t regionY = result.getRegion().getLowerCorner().getY() / regionSideLength;
		uint16_t regionZ = result.getRegion().getLowerCorner().getZ() / regionSideLength;

		std::uint32_t uRegionTimeStamp = volumeChangeTracker->getLastModifiedTimeForRegion(regionX, regionY, regionZ);
		if(uRegionTimeStamp > result.m_uTimeStamp)
		{
			// The volume has changed since the command to generate this mesh was issued.
			// Just ignore it, and a correct version should be along soon...
			return;
		}

		uploadSurfaceMesh(result.getSurfaceMesh(), result.getRegion());
	}

	void Map::uploadSurfaceMesh(const SurfaceMesh& mesh, PolyVox::Region region)
	{
		bool bSimulatePhysics = qApp->settings()->value("Physics/SimulatePhysics", false).toBool();
		std::uint16_t regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();

		//Determine where it came from
		uint16_t regionX = region.getLowerCorner().getX() / regionSideLength;
		uint16_t regionY = region.getLowerCorner().getY() / regionSideLength;
		uint16_t regionZ = region.getLowerCorner().getZ() / regionSideLength;

		/*uint32_t uRegionTimeStamp = volumeChangeTracker->getLastModifiedTimeForRegion(regionX, regionY, regionZ);
		if(uRegionTimeStamp > result.m_uTimeStamp)
		{
			// The volume has changed since the command to generate this mesh was issued.
			// Just ignore it, and a correct version should be along soon...
			return;
		}*/

		//Create a MapRegion for that location if we don't have one already
		MapRegion* pMapRegion = m_volMapRegions->getVoxelAt(regionX, regionY, regionZ);
		if(pMapRegion == 0)
		{
			pMapRegion = new MapRegion(this, region.getLowerCorner());
			m_volMapRegions->setVoxelAt(regionX, regionY, regionZ, pMapRegion);
		}

		//Clear any previous geometry
		pMapRegion->removeAllSurfacePatchRenderables();

		//Get the SurfaceMesh and check it's valid
		SurfaceMesh meshWhole = mesh;
		if(meshWhole.isEmpty() == false)
		{			
			//The SurfaceMesh needs to be broken into pieces - one for each material. Iterate over the mateials...
			for(std::map< std::string, std::set<uint8_t> >::iterator iter = m_mapMaterialIds.begin(); iter != m_mapMaterialIds.end(); iter++)
			{
				//Get the properties
				std::string materialName = iter->first;
				std::set<std::uint8_t> voxelValues = iter->second;

				//Extract the part of the InexedSurfacePatch which corresponds to that material
				shared_ptr<SurfaceMesh> meshSubset = meshWhole.extractSubset(voxelValues);

				//And add it to the MapRegion
				pMapRegion->addSurfacePatchRenderable(materialName, *meshSubset);
			}

			//If we are simulating physics...
			/*if(bSimulatePhysics)
			{
				//Update the physics geometry
				pMapRegion->setPhysicsData(*(meshWhole.get()));
			}*/
		}

		m_volRegionBeingProcessed->setVoxelAt(regionX,regionY,regionZ,false);
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