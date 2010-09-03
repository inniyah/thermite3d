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

#include "Scriptable/Volume.h"

#include "Application.h"
#include "VolumeManager.h"
#include "Utility.h"

#include "VolumeChangeTracker.h"

#include "MaterialDensityPair.h"

#include "SurfaceMeshExtractionTask.h"
#include "SurfaceMeshDecimationTask.h"
#include "TaskProcessorThread.h"

#include <QSettings>
#include <QThreadPool>

//using namespace Ogre;
using namespace PolyVox;

namespace Thermite
{
	TaskProcessorThread* Volume::m_backgroundThread = 0;

	Volume::Volume(QObject* parent)
		:Object(parent)
	{
		m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(1);
		m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(2);
		m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(3);
		m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(4);
		m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(5);
		m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(6);
		m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(7);
		m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(8);
	}

	Volume::~Volume(void)
	{
	}

	void Volume::initialise(void)
	{
		int regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();

		volumeChangeTracker = new VolumeChangeTracker<MaterialDensityPair44>(m_pPolyVoxVolume.get(), regionSideLength);
		volumeChangeTracker->setAllRegionsModified();

		int volumeWidthInRegions = volumeChangeTracker->getWrappedVolume()->getWidth() / regionSideLength;
		int volumeHeightInRegions = volumeChangeTracker->getWrappedVolume()->getHeight() / regionSideLength;
		int volumeDepthInRegions = volumeChangeTracker->getWrappedVolume()->getDepth() / regionSideLength;

		uint32_t dimensions[3] = {volumeWidthInRegions, volumeHeightInRegions, volumeDepthInRegions}; // Array dimensions
		m_volRegionTimeStamps.resize(dimensions); std::fill(m_volRegionTimeStamps.getRawData(), m_volRegionTimeStamps.getRawData() + m_volRegionTimeStamps.getNoOfElements(), 0);
		m_volLatestMeshTimeStamps.resize(dimensions); std::fill(m_volLatestMeshTimeStamps.getRawData(), m_volLatestMeshTimeStamps.getRawData() + m_volLatestMeshTimeStamps.getNoOfElements(), 0);
		m_volSurfaceMeshes.resize(dimensions); std::fill(m_volSurfaceMeshes.getRawData(), m_volSurfaceMeshes.getRawData() + m_volSurfaceMeshes.getNoOfElements(), (PolyVox::SurfaceMesh*)0);
		m_volRegionBeingProcessed.resize(dimensions); std::fill(m_volRegionBeingProcessed.getRawData(), m_volRegionBeingProcessed.getRawData() + m_volRegionBeingProcessed.getNoOfElements(), 0);
		m_volSurfaceDecimators.resize(dimensions); std::fill(m_volSurfaceDecimators.getRawData(), m_volSurfaceDecimators.getRawData() + m_volSurfaceDecimators.getNoOfElements(), (Thermite::SurfaceMeshDecimationTask*)0);

		if(!m_backgroundThread)
		{
			m_backgroundThread = new TaskProcessorThread;
			m_backgroundThread->setPriority(QThread::LowestPriority);
			m_backgroundThread->start();
		}
	}

	void Volume::updatePolyVoxGeometry(const QVector3D& cameraPos)
	{
		if(m_pPolyVoxVolume)
		{		
			//Some values we'll need later.
			std::uint16_t regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();
			std::uint16_t halfRegionSideLength = regionSideLength / 2;
			std::uint16_t volumeWidthInRegions = m_pPolyVoxVolume->getWidth() / regionSideLength;
			std::uint16_t volumeHeightInRegions = m_pPolyVoxVolume->getHeight() / regionSideLength;
			std::uint16_t volumeDepthInRegions = m_pPolyVoxVolume->getDepth() / regionSideLength;

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
						QVector3D centre(centreX, centreY, centreZ);
						double distanceFromCameraSquared = (cameraPos - centre).lengthSquared();

						std::uint32_t uRegionTimeStamp = volumeChangeTracker->getLastModifiedTimeForRegion(regionX, regionY, regionZ);
						if(uRegionTimeStamp > m_volRegionTimeStamps[regionX][regionY][regionZ])
						{
							m_volRegionBeingProcessed[regionX][regionY][regionZ];

							//Convert to a real PolyVox::Region
							Vector3DInt16 v3dLowerCorner(firstX,firstY,firstZ);
							Vector3DInt16 v3dUpperCorner(lastX,lastY,lastZ);
							PolyVox::Region region(v3dLowerCorner, v3dUpperCorner);
							region.cropTo(volumeChangeTracker->getWrappedVolume()->getEnclosingRegion());

							//The prioirty ensures that the surfaces for regions close to the
							//camera get extracted before those which are distant from the camera.
							std::uint32_t uPriority = std::numeric_limits<std::uint32_t>::max() - static_cast<std::uint32_t>(distanceFromCameraSquared);

							//Extract the region
							SurfaceExtractorTaskData taskData(m_pPolyVoxVolume.get(), region, uRegionTimeStamp);
							SurfaceMeshExtractionTask* surfaceMeshExtractionTask = new SurfaceMeshExtractionTask(taskData);
							QObject::connect(surfaceMeshExtractionTask, SIGNAL(finished(SurfaceExtractorTaskData)), this, SLOT(uploadSurfaceExtractorResult(SurfaceExtractorTaskData)), Qt::QueuedConnection);
							QThreadPool::globalInstance()->start(surfaceMeshExtractionTask, uPriority);

							//Indicate that we've processed this region
							m_volRegionTimeStamps[regionX][regionY][regionZ] = volumeChangeTracker->getLastModifiedTimeForRegion(regionX, regionY, regionZ);
						}
					}
				}
			}
		}
	}

	void Volume::surfaceExtractionFinished(SurfaceExtractorTaskData result)
	{
		std::uint16_t regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();

		//Determine where it came from
		uint16_t regionX = result.getRegion().getLowerCorner().getX() / regionSideLength;
		uint16_t regionY = result.getRegion().getLowerCorner().getY() / regionSideLength;
		uint16_t regionZ = result.getRegion().getLowerCorner().getZ() / regionSideLength;

		SurfaceMesh* pMesh = new SurfaceMesh;
		*pMesh = result.m_meshResult;
		m_volSurfaceMeshes[regionX][regionY][regionZ] = pMesh;
	}

	void Volume::uploadSurfaceExtractorResult(SurfaceExtractorTaskData result)
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

		SurfaceMesh* pMesh = new SurfaceMesh;
		*pMesh = result.m_meshResult;
		pMesh->m_Region = result.getRegion();
		m_volSurfaceMeshes[regionX][regionY][regionZ] = pMesh;
		m_volLatestMeshTimeStamps[regionX][regionY][regionZ] = getTimeStamp();

		//uploadSurfaceMesh(result.getSurfaceMesh(), result.getRegion());


		SurfaceMeshDecimationTask* pOldSurfaceDecimator = m_volSurfaceDecimators[regionX][regionY][regionZ];

		m_backgroundThread->removeTask(pOldSurfaceDecimator);

		SurfaceMeshDecimationTask* surfaceMeshDecimationTask = new SurfaceMeshDecimationTask(result);
		QObject::connect(surfaceMeshDecimationTask, SIGNAL(finished(SurfaceExtractorTaskData)), this, SLOT(uploadSurfaceDecimatorResult(SurfaceExtractorTaskData)), Qt::QueuedConnection);

		m_volSurfaceDecimators[regionX][regionY][regionZ] = surfaceMeshDecimationTask;

		m_backgroundThread->addTask(surfaceMeshDecimationTask);
	}

	void Volume::uploadSurfaceDecimatorResult(SurfaceExtractorTaskData result)
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

		SurfaceMesh* pMesh = new SurfaceMesh;
		*pMesh = result.m_meshResult;
		m_volSurfaceMeshes[regionX][regionY][regionZ] = pMesh;
		m_volLatestMeshTimeStamps[regionX][regionY][regionZ] = getTimeStamp();

		//uploadSurfaceMesh(result.getSurfaceMesh(), result.getRegion());
	}	

	void  Volume::createSphereAt(QVector3D centre, float radius, int value, bool bPaintMode)
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
					if(m_volRegionBeingProcessed[xCt][yCt][zCt])
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

	QVector3D Volume::getRayVolumeIntersection(QVector3D rayOrigin, const QVector3D& rayDir)
	{
		//Initialise to failure
		/*std::pair<bool, QVector3D> result;
		result.first = false;
		result.second = QVector3D(0,0,0);*/

		QVector3D result = QVector3D(0,0,0);

		//Ensure the voume is valid
		PolyVox::Volume<MaterialDensityPair44>* pVolume = m_pPolyVoxVolume.get();
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

	bool Volume::loadFromFile(const QString& filename)
	{
		m_pPolyVoxVolume = VolumeManager::getSingletonPtr()->load(filename.toStdString(), "General")->getVolume();
		return true;
	}
}