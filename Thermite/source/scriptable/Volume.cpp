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

#include "scriptable/Volume.h"

#include "scriptable/Globals.h"

#include "Application.h"
#include "Log.h"

#include "VolumeManager.h"

#include "MaterialDensityPair.h"


#include "SurfaceMeshExtractionTask.h"
#include "SurfaceMeshDecimationTask.h"
#include "TaskProcessorThread.h"

#include <QSettings>
#include <QThreadPool>

using namespace PolyVox;

namespace Thermite
{
	TaskProcessorThread* Volume::m_backgroundThread = 0;

	Volume::Volume(QObject* parent)
		:Object(parent)
	{
		/*m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(1);
		m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(2);
		m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(3);
		m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(4);
		m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(5);
		m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(6);
		m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(7);
		m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(8);*/

		m_mapMaterialIds["ColouredCubicVoxel"].insert(1);
		m_mapMaterialIds["ColouredCubicVoxel"].insert(2);
		m_mapMaterialIds["ColouredCubicVoxel"].insert(3);
		m_mapMaterialIds["ColouredCubicVoxel"].insert(4);
		m_mapMaterialIds["ColouredCubicVoxel"].insert(5);
		m_mapMaterialIds["ColouredCubicVoxel"].insert(6);
		m_mapMaterialIds["ColouredCubicVoxel"].insert(7);
		m_mapMaterialIds["ColouredCubicVoxel"].insert(8);
	}

	Volume::~Volume(void)
	{
	}

	void Volume::setPolyVoxVolume(polyvox_shared_ptr< PolyVox::Volume<PolyVox::MaterialDensityPair44> > pPolyVoxVolume, uint16_t regionSideLength)
	{
		m_pPolyVoxVolume = pPolyVoxVolume;
		mRegionSideLength = regionSideLength;		

		mVolumeWidthInRegions = m_pPolyVoxVolume->getWidth() / regionSideLength;
		mVolumeHeightInRegions = m_pPolyVoxVolume->getHeight() / regionSideLength;
		mVolumeDepthInRegions = m_pPolyVoxVolume->getDepth() / regionSideLength;

		uint32_t dimensions[3] = {mVolumeWidthInRegions, mVolumeHeightInRegions, mVolumeDepthInRegions}; // Array dimensions
		mLastModifiedArray.resize(dimensions); std::fill(mLastModifiedArray.getRawData(), mLastModifiedArray.getRawData() + mLastModifiedArray.getNoOfElements(), globals.timeStamp());
		mExtractionStartedArray.resize(dimensions); std::fill(mExtractionStartedArray.getRawData(), mExtractionStartedArray.getRawData() + mExtractionStartedArray.getNoOfElements(), 0);
		mExtractionFinishedArray.resize(dimensions); std::fill(mExtractionFinishedArray.getRawData(), mExtractionFinishedArray.getRawData() + mExtractionFinishedArray.getNoOfElements(), 0);
		m_volSurfaceMeshes.resize(dimensions); std::fill(m_volSurfaceMeshes.getRawData(), m_volSurfaceMeshes.getRawData() + m_volSurfaceMeshes.getNoOfElements(), (PolyVox::SurfaceMesh*)0);
		mRegionBeingExtracted.resize(dimensions); std::fill(mRegionBeingExtracted.getRawData(), mRegionBeingExtracted.getRawData() + mRegionBeingExtracted.getNoOfElements(), 0);
		m_volSurfaceDecimators.resize(dimensions); std::fill(m_volSurfaceDecimators.getRawData(), m_volSurfaceDecimators.getRawData() + m_volSurfaceDecimators.getNoOfElements(), (Thermite::SurfaceMeshDecimationTask*)0);
	}

	void Volume::initialise(void)
	{
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
			//Iterate over each region
			for(std::uint16_t regionZ = 0; regionZ < mVolumeDepthInRegions; ++regionZ)
			{		
				for(std::uint16_t regionY = 0; regionY < mVolumeHeightInRegions; ++regionY)
				{
					for(std::uint16_t regionX = 0; regionX < mVolumeWidthInRegions; ++regionX)
					{
						//Compute the extents of the current region
						const std::uint16_t firstX = regionX * mRegionSideLength;
						const std::uint16_t firstY = regionY * mRegionSideLength;
						const std::uint16_t firstZ = regionZ * mRegionSideLength;

						const std::uint16_t lastX = firstX + mRegionSideLength;
						const std::uint16_t lastY = firstY + mRegionSideLength;
						const std::uint16_t lastZ = firstZ + mRegionSideLength;	

						const uint16_t halfRegionSideLength = mRegionSideLength / 2;
						const float centreX = firstX + halfRegionSideLength;
						const float centreY = firstY + halfRegionSideLength;
						const float centreZ = firstZ + halfRegionSideLength;

						//The regions distance from the camera is used for prioritizing surface extraction
						QVector3D centre(centreX, centreY, centreZ);
						double distanceFromCameraSquared = (cameraPos - centre).lengthSquared();

						if(mLastModifiedArray[regionX][regionY][regionZ] > mExtractionStartedArray[regionX][regionY][regionZ])
						{
							//Make a note that we're extracting this region - this is used to block other updates.
							mRegionBeingExtracted[regionX][regionY][regionZ] = true;

							//Convert to a real PolyVox::Region
							Vector3DInt16 v3dLowerCorner(firstX,firstY,firstZ);
							Vector3DInt16 v3dUpperCorner(lastX,lastY,lastZ);
							PolyVox::Region region(v3dLowerCorner, v3dUpperCorner);
							region.cropTo(m_pPolyVoxVolume->getEnclosingRegion());

							//The prioirty ensures that the surfaces for regions close to the
							//camera get extracted before those which are distant from the camera.
							std::uint32_t uPriority = std::numeric_limits<std::uint32_t>::max() - static_cast<std::uint32_t>(distanceFromCameraSquared);

							//Extract the region
							SurfaceMeshExtractionTask* surfaceMeshExtractionTask = new SurfaceMeshExtractionTask(m_pPolyVoxVolume.get(), region, mLastModifiedArray[regionX][regionY][regionZ]);
							surfaceMeshExtractionTask->setAutoDelete(false);
							QObject::connect(surfaceMeshExtractionTask, SIGNAL(finished(SurfaceMeshExtractionTask*)), this, SLOT(uploadSurfaceExtractorResult(SurfaceMeshExtractionTask*)), Qt::QueuedConnection);
							QThreadPool::globalInstance()->start(surfaceMeshExtractionTask, uPriority);

							//Indicate that we've processed this region
							mExtractionStartedArray[regionX][regionY][regionZ] = mLastModifiedArray[regionX][regionY][regionZ];
						}
					}
				}
			}
		}
	}

	void Volume::uploadSurfaceExtractorResult(SurfaceMeshExtractionTask* pTask)
	{
		SurfaceMesh* pMesh = &(pTask->m_meshResult);

		//Determine where it came from
		uint16_t regionX = pMesh->m_Region.getLowerCorner().getX() / mRegionSideLength;
		uint16_t regionY = pMesh->m_Region.getLowerCorner().getY() / mRegionSideLength;
		uint16_t regionZ = pMesh->m_Region.getLowerCorner().getZ() / mRegionSideLength;

		std::uint32_t uRegionTimeStamp = mLastModifiedArray[regionX][regionY][regionZ];
		if(uRegionTimeStamp > pTask->m_uTimeStamp)
		{
			// The volume has changed since the command to generate this mesh was issued.
			// Just ignore it, and a correct version should be along soon...
			return;
		}
		
		//pMesh->m_Region = result.getRegion();
		m_volSurfaceMeshes[regionX][regionY][regionZ] = pMesh;
		mExtractionFinishedArray[regionX][regionY][regionZ] = globals.timeStamp();

		//uploadSurfaceMesh(result.getSurfaceMesh(), result.getRegion());

		mRegionBeingExtracted[regionX][regionY][regionZ] = false;


		SurfaceMeshDecimationTask* pOldSurfaceDecimator = m_volSurfaceDecimators[regionX][regionY][regionZ];

		m_backgroundThread->removeTask(pOldSurfaceDecimator);

		SurfaceMeshDecimationTask* surfaceMeshDecimationTask = new SurfaceMeshDecimationTask(pMesh, pTask->m_uTimeStamp);
		surfaceMeshDecimationTask->setAutoDelete(false);
		QObject::connect(surfaceMeshDecimationTask, SIGNAL(finished(SurfaceMeshDecimationTask*)), this, SLOT(uploadSurfaceDecimatorResult(SurfaceMeshDecimationTask*)), Qt::QueuedConnection);

		m_volSurfaceDecimators[regionX][regionY][regionZ] = surfaceMeshDecimationTask;

		//m_backgroundThread->addTask(surfaceMeshDecimationTask);
	}

	void Volume::uploadSurfaceDecimatorResult(SurfaceMeshDecimationTask* pTask)
	{
		SurfaceMesh* pMesh = pTask->mMesh;

		//Determine where it came from
		uint16_t regionX = pMesh->m_Region.getLowerCorner().getX() / mRegionSideLength;
		uint16_t regionY = pMesh->m_Region.getLowerCorner().getY() / mRegionSideLength;
		uint16_t regionZ = pMesh->m_Region.getLowerCorner().getZ() / mRegionSideLength;

		std::uint32_t uRegionTimeStamp = mLastModifiedArray[regionX][regionY][regionZ];

		if(uRegionTimeStamp > pTask->m_uTimeStamp)
		{
			// The volume has changed since the command to generate this mesh was issued.
			// Just ignore it, and a correct version should be along soon...
			return;
		}

		
		m_volSurfaceMeshes[regionX][regionY][regionZ] = pMesh;
		mExtractionFinishedArray[regionX][regionY][regionZ] = globals.timeStamp();

		delete pTask;

		//uploadSurfaceMesh(result.getSurfaceMesh(), result.getRegion());
	}	

	bool Volume::isRegionBeingExtracted(const Region& regionToTest)
	{
		//This is ugly, but basically we are making sure that we do not modify part of the volume of the mesh is currently
		//being regenerated for that part. This is to avoid 'queing up' a whole bunch of surface exreaction commands for 
		//the same region, only to have them rejected because the time stamp has changed again since they were issued.

		//At this point it probably makes sense to pull the VolumeChangeTracker from PolyVox into Thermite and have it
		//handle these checks as well.

		//Longer term, it might be interesting to introduce a 'ModifyVolumeCommand' which can be issued to runn on seperate threads.
		//We could then schedule these so that all the ones for a given region are processed before we issue the extract surface command
		//for that region.

		//Should shift not divide!
		const std::uint16_t firstRegionX = regionToTest.getLowerCorner().getX() / mRegionSideLength;
		const std::uint16_t firstRegionY = regionToTest.getLowerCorner().getY() / mRegionSideLength;
		const std::uint16_t firstRegionZ = regionToTest.getLowerCorner().getZ() / mRegionSideLength;

		const std::uint16_t lastRegionX = regionToTest.getUpperCorner().getX() / mRegionSideLength;
		const std::uint16_t lastRegionY = regionToTest.getUpperCorner().getY() / mRegionSideLength;
		const std::uint16_t lastRegionZ = regionToTest.getUpperCorner().getZ() / mRegionSideLength;

		for(std::uint16_t zCt = firstRegionZ; zCt <= lastRegionZ; zCt++)
		{
			for(std::uint16_t yCt = firstRegionY; yCt <= lastRegionY; yCt++)
			{
				for(std::uint16_t xCt = firstRegionX; xCt <= lastRegionX; xCt++)
				{
					//volRegionLastModified->setVoxelAt(xCt,yCt,zCt,m_uCurrentTime);
					if(mRegionBeingExtracted[xCt][yCt][zCt])
					{
						return true;
					}
				}
			}
		}

		return false;
	}

	void Volume::updateLastModifedArray(const Region& regionToTest)
	{
		const std::uint16_t firstRegionX = regionToTest.getLowerCorner().getX() / mRegionSideLength;
		const std::uint16_t firstRegionY = regionToTest.getLowerCorner().getY() / mRegionSideLength;
		const std::uint16_t firstRegionZ = regionToTest.getLowerCorner().getZ() / mRegionSideLength;

		const std::uint16_t lastRegionX = regionToTest.getUpperCorner().getX() / mRegionSideLength;
		const std::uint16_t lastRegionY = regionToTest.getUpperCorner().getY() / mRegionSideLength;
		const std::uint16_t lastRegionZ = regionToTest.getUpperCorner().getZ() / mRegionSideLength;

		for(uint16_t zCt = firstRegionZ; zCt <= lastRegionZ; zCt++)
		{
			for(uint16_t yCt = firstRegionY; yCt <= lastRegionY; yCt++)
			{
				for(uint16_t xCt = firstRegionX; xCt <= lastRegionX; xCt++)
				{
					//volRegionLastModified->setVoxelAt(xCt,yCt,zCt,m_uCurrentTime);
					mLastModifiedArray[xCt][yCt][zCt] = globals.timeStamp();
				}
			}
		}
	}

	void Volume::createSphereAt(QVector3D centre, float radius, int value, bool bPaintMode)
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

		lastX = std::min(lastX,int(m_pPolyVoxVolume->getWidth()-1));
		lastY = std::min(lastY,int(m_pPolyVoxVolume->getHeight()-1));
		lastZ = std::min(lastZ,int(m_pPolyVoxVolume->getDepth()-1));

		PolyVox::Region regionToLock = PolyVox::Region(PolyVox::Vector3DInt16(firstX, firstY, firstZ), PolyVox::Vector3DInt16(lastX, lastY, lastZ));

		if(isRegionBeingExtracted(regionToLock))
		{
			//Just skip doing anything - volume will not be modified. Try again later...
			return;
		}

		for(int z = firstZ; z <= lastZ; ++z)
		{
			for(int y = firstY; y <= lastY; ++y)
			{
				for(int x = firstX; x <= lastX; ++x)
				{
					if((centre - QVector3D(x,y,z)).lengthSquared() <= radiusSquared)
					{
						MaterialDensityPair44 currentValue = m_pPolyVoxVolume->getVoxelAt(x,y,z);
						currentValue.setDensity(MaterialDensityPair44::getMaxDensity());
						m_pPolyVoxVolume->setVoxelAt(x,y,z,currentValue);
					}
				}
			}
		}

		updateLastModifedArray(regionToLock);

		qApp->getLogByName("Thermite")->logMessage("Volume was modified", QtOgre::LL_INFO);
	}

	void Volume::createCuboidAt(QVector3D centre, QVector3D dimensions, int material, int density, bool bPaintMode)
	{
		dimensions /= 2.0f;

		int firstX = static_cast<int>(std::floor(centre.x() - dimensions.x()));
		int firstY = static_cast<int>(std::floor(centre.y() - dimensions.y()));
		int firstZ = static_cast<int>(std::floor(centre.z() - dimensions.z()));

		int lastX = static_cast<int>(std::ceil(centre.x() + dimensions.x()));
		int lastY = static_cast<int>(std::ceil(centre.y() + dimensions.y()));
		int lastZ = static_cast<int>(std::ceil(centre.z() + dimensions.z()));

		//Check bounds
		firstX = std::max(firstX,0);
		firstY = std::max(firstY,0);
		firstZ = std::max(firstZ,0);

		lastX = std::min(lastX,int(m_pPolyVoxVolume->getWidth()-1));
		lastY = std::min(lastY,int(m_pPolyVoxVolume->getHeight()-1));
		lastZ = std::min(lastZ,int(m_pPolyVoxVolume->getDepth()-1));

		PolyVox::Region regionToLock = PolyVox::Region(PolyVox::Vector3DInt16(firstX, firstY, firstZ), PolyVox::Vector3DInt16(lastX, lastY, lastZ));

		if(isRegionBeingExtracted(regionToLock))
		{
			//Just skip doing anything - volume will not be modified. Try again later...
			return;
		}

		for(int z = firstZ; z <= lastZ; ++z)
		{
			for(int y = firstY; y <= lastY; ++y)
			{
				for(int x = firstX; x <= lastX; ++x)
				{
					MaterialDensityPair44 value(material, density);
					m_pPolyVoxVolume->setVoxelAt(x,y,z,value);
				}
			}
		}

		updateLastModifedArray(regionToLock);

		qApp->getLogByName("Thermite")->logMessage("Volume was modified", QtOgre::LL_INFO);
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
		uint16_t regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();
		polyvox_shared_ptr< PolyVox::Volume<PolyVox::MaterialDensityPair44> > pPolyVoxVolume = VolumeManager::getSingletonPtr()->load(filename.toStdString(), "General")->getVolume();
		setPolyVoxVolume(pPolyVoxVolume, regionSideLength);
		return true;
	}
}
