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

#include "scriptable/Volume.h"

#include "scriptable/Globals.h"

#include "Application.h"
#include "Log.h"

#include "VolumeManager.h"

#include "Material.h"

#include "Raycast.h"
#include "VolumeResampler.h"

#include "Perlin.h"
#include "FindPathTask.h"
#include "AmbientOcclusionTask.h"
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
		,m_pPolyVoxVolume(0)
		,mVolumeWidthInRegions(0)
		,mVolumeHeightInRegions(0)
		,mVolumeDepthInRegions(0)
		,mMultiThreadedSurfaceExtraction(true)
	{
		/*m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(1);
		m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(2);
		m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(3);
		m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(4);
		m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(5);
		m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(6);
		m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(7);
		m_mapMaterialIds["ShadowMapReceiverForWorldMaterial"].insert(8);*/

		for(int ct = 1; ct < 256; ct++)
		{
			m_mapMaterialIds["ColouredCubicVoxel"].insert(ct);
		}

		if(!m_backgroundThread)
		{
			m_backgroundThread = new TaskProcessorThread;
			m_backgroundThread->setPriority(QThread::LowestPriority);
			m_backgroundThread->start();
		}
	}

	Volume::~Volume(void)
	{
	}

	void Volume::setPolyVoxVolume(PolyVox::Volume<PolyVox::Material8>* pPolyVoxVolume, uint16_t regionSideLength)
	{
		m_pPolyVoxVolume = pPolyVoxVolume;
		mRegionSideLength = regionSideLength;		

		mVolumeWidthInRegions = m_pPolyVoxVolume->getWidth() / regionSideLength;
		mVolumeHeightInRegions = m_pPolyVoxVolume->getHeight() / regionSideLength;
		mVolumeDepthInRegions = m_pPolyVoxVolume->getDepth() / regionSideLength;

		mLightRegionSideLength = qApp->settings()->value("Engine/LightRegionSideLength", 32).toInt();
		mVolumeWidthInLightRegions = m_pPolyVoxVolume->getWidth() / mLightRegionSideLength;
		mVolumeHeightInLightRegions = m_pPolyVoxVolume->getHeight() / mLightRegionSideLength;
		mVolumeDepthInLightRegions = m_pPolyVoxVolume->getDepth() / mLightRegionSideLength;

		uint32_t dimensions[3] = {mVolumeWidthInRegions, mVolumeHeightInRegions, mVolumeDepthInRegions}; // Array dimensions
		mLastModifiedArray.resize(dimensions); std::fill(mLastModifiedArray.getRawData(), mLastModifiedArray.getRawData() + mLastModifiedArray.getNoOfElements(), globals.timeStamp());
		mExtractionStartedArray.resize(dimensions); std::fill(mExtractionStartedArray.getRawData(), mExtractionStartedArray.getRawData() + mExtractionStartedArray.getNoOfElements(), 0);
		mExtractionFinishedArray.resize(dimensions); std::fill(mExtractionFinishedArray.getRawData(), mExtractionFinishedArray.getRawData() + mExtractionFinishedArray.getNoOfElements(), 0);
		m_volSurfaceMeshes.resize(dimensions); std::fill(m_volSurfaceMeshes.getRawData(), m_volSurfaceMeshes.getRawData() + m_volSurfaceMeshes.getNoOfElements(), (SurfaceMesh<PositionMaterial>*)0);
		mRegionBeingExtracted.resize(dimensions); std::fill(mRegionBeingExtracted.getRawData(), mRegionBeingExtracted.getRawData() + mRegionBeingExtracted.getNoOfElements(), 0);
		m_volSurfaceDecimators.resize(dimensions); std::fill(m_volSurfaceDecimators.getRawData(), m_volSurfaceDecimators.getRawData() + m_volSurfaceDecimators.getNoOfElements(), (Thermite::SurfaceMeshDecimationTask*)0);

		uint32_t lightDimensions[3] = {mVolumeWidthInLightRegions, mVolumeHeightInLightRegions, mVolumeDepthInLightRegions}; // Array dimensions
		mLightLastModifiedArray.resize(lightDimensions); std::fill(mLightLastModifiedArray.getRawData(), mLightLastModifiedArray.getRawData() + mLightLastModifiedArray.getNoOfElements(), globals.timeStamp());
		mLightingStartedArray.resize(lightDimensions); std::fill(mLightingStartedArray.getRawData(), mLightingStartedArray.getRawData() + mLightingStartedArray.getNoOfElements(), 0);
		mLightingFinishedArray.resize(lightDimensions); std::fill(mLightingFinishedArray.getRawData(), mLightingFinishedArray.getRawData() + mLightingFinishedArray.getNoOfElements(), 0);

		/*PolyVox::Volume<Material8> subSampledVolume(m_pPolyVoxVolume->getWidth() / 2, m_pPolyVoxVolume->getHeight() / 2, m_pPolyVoxVolume->getDepth() / 2);
		VolumeResampler<Material8> volumeResampler(m_pPolyVoxVolume, &subSampledVolume);
		volumeResampler.execute();*/

		//Ambient Occlusion
		mAmbientOcclusionVolume.resize(ArraySizes(64)(16)(64));
		std::fill(mAmbientOcclusionVolume.getRawData(), mAmbientOcclusionVolume.getRawData() + mAmbientOcclusionVolume.getNoOfElements(), 0);

		QTime time;
		time.start();
		VolumeResampler<Material8> volumeResampler(m_pPolyVoxVolume, &mAmbientOcclusionVolume, m_pPolyVoxVolume->getEnclosingRegion(), mLightRegionSideLength);
		volumeResampler.execute();
		qDebug() << "Lighting time = " << time.elapsed();
	}

	void Volume::initialise(void)
	{		
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

						/*const*/ std::uint16_t lastX = firstX + mRegionSideLength;
						/*const*/ std::uint16_t lastY = firstY + mRegionSideLength;
						/*const*/ std::uint16_t lastZ = firstZ + mRegionSideLength;	

						//NOTE: When using the CubicSurfaceExtractor the regions do not touch
						//in the same way as the MC surface extractor. Adjust for that here.
						--lastX;
						--lastY;
						--lastZ;	

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
							SurfaceMeshExtractionTask* surfaceMeshExtractionTask = new SurfaceMeshExtractionTask(m_pPolyVoxVolume, region, mLastModifiedArray[regionX][regionY][regionZ]);
							surfaceMeshExtractionTask->setAutoDelete(false);
							QObject::connect(surfaceMeshExtractionTask, SIGNAL(finished(SurfaceMeshExtractionTask*)), this, SLOT(uploadSurfaceExtractorResult(SurfaceMeshExtractionTask*)), Qt::QueuedConnection);
							if(mMultiThreadedSurfaceExtraction)
							{
								QThreadPool::globalInstance()->start(surfaceMeshExtractionTask, uPriority);
							}
							else
							{
								surfaceMeshExtractionTask->run();
							}

							//Indicate that we've processed this region
							mExtractionStartedArray[regionX][regionY][regionZ] = mLastModifiedArray[regionX][regionY][regionZ];
						}
					}
				}
			}

			//NOTE: Technically I don't believe it is correct to just update the ambient lighting for blocks which have
			//changed - the ambient light of neighbouring blocks could change as well. So we should probably take this
			//'modified' array and dilate it by one. But in practive I haven't seen any artifacts yet when not doing this.
			for(std::uint16_t regionZ = 0; regionZ < mVolumeDepthInLightRegions; ++regionZ)
			{		
				for(std::uint16_t regionY = 0; regionY < mVolumeHeightInLightRegions; ++regionY)
				{
					for(std::uint16_t regionX = 0; regionX < mVolumeWidthInLightRegions; ++regionX)
					{
						if(mLightLastModifiedArray[regionX][regionY][regionZ] > mLightingStartedArray[regionX][regionY][regionZ])
						{
							//Compute the extents of the current region
							const std::uint16_t firstX = regionX * mLightRegionSideLength;
							const std::uint16_t firstY = regionY * mLightRegionSideLength;
							const std::uint16_t firstZ = regionZ * mLightRegionSideLength;

							/*const*/ std::uint16_t lastX = firstX + mLightRegionSideLength;
							/*const*/ std::uint16_t lastY = firstY + mLightRegionSideLength;
							/*const*/ std::uint16_t lastZ = firstZ + mLightRegionSideLength;	
							
							//For lighting the regions touch like cubic rather than MC surface extractor.
							--lastX;
							--lastY;
							--lastZ;	

							Vector3DInt16 v3dLowerCorner(firstX,firstY,firstZ);
							Vector3DInt16 v3dUpperCorner(lastX,lastY,lastZ);
							PolyVox::Region region(v3dLowerCorner, v3dUpperCorner);
							region.cropTo(m_pPolyVoxVolume->getEnclosingRegion());

							/*VolumeResampler<Material8> volumeResampler(m_pPolyVoxVolume, &mAmbientOcclusionVolume, region);
							volumeResampler.execute();*/

							AmbientOcclusionTask* ambientOcclusionTask = new AmbientOcclusionTask(m_pPolyVoxVolume, &mAmbientOcclusionVolume,region, mLightLastModifiedArray[regionX][regionY][regionZ], mLightRegionSideLength);
							ambientOcclusionTask->setAutoDelete(false);
							QObject::connect(ambientOcclusionTask, SIGNAL(finished(AmbientOcclusionTask*)), this, SLOT(uploadAmbientOcclusionResult(AmbientOcclusionTask*)), Qt::QueuedConnection);
							if(mMultiThreadedSurfaceExtraction)
							{
								QThreadPool::globalInstance()->start(ambientOcclusionTask);
							}
							else
							{
								ambientOcclusionTask->run();
							}

							mLightingStartedArray[regionX][regionY][regionZ] = mLightLastModifiedArray[regionX][regionY][regionZ];
						}
					}
				}
			}
			setModified(true);
		}
	}

	void Volume::uploadAmbientOcclusionResult(AmbientOcclusionTask* pTask)
	{
		//PolyVox::Array<3, uint8_t>* pAmbientOcclusionVolume = pTask->mAmbientOcclusionVolume;

		//Determine where it came from
		uint16_t regionX = pTask->m_regToProcess.getLowerCorner().getX() / mLightRegionSideLength;
		uint16_t regionY = pTask->m_regToProcess.getLowerCorner().getY() / mLightRegionSideLength;
		uint16_t regionZ = pTask->m_regToProcess.getLowerCorner().getZ() / mLightRegionSideLength;

		std::uint32_t uRegionTimeStamp = mLightLastModifiedArray[regionX][regionY][regionZ];
		if(uRegionTimeStamp > pTask->m_uTimeStamp)
		{
			// The volume has changed since the command to generate this mesh was issued.
			// Just ignore it, and a correct version should be along soon...
			return;
		}

		mLightingFinishedArray[regionX][regionY][regionZ] = globals.timeStamp();
	}

	void Volume::uploadSurfaceExtractorResult(SurfaceMeshExtractionTask* pTask)
	{
		SurfaceMesh<PositionMaterial>* pMesh = &(pTask->m_meshResult);

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
		SurfaceMesh<PositionMaterial>* pMesh = pTask->mMesh;

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

	bool Volume::isRegionBeingExtracted(const PolyVox::Region& regionToTest)
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

	void Volume::updateLastModifedArray(const PolyVox::Region& regionToTest)
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

		const std::uint16_t firstLightRegionX = regionToTest.getLowerCorner().getX() / mLightRegionSideLength;
		const std::uint16_t firstLightRegionY = regionToTest.getLowerCorner().getY() / mLightRegionSideLength;
		const std::uint16_t firstLightRegionZ = regionToTest.getLowerCorner().getZ() / mLightRegionSideLength;

		const std::uint16_t lastLightRegionX = regionToTest.getUpperCorner().getX() / mLightRegionSideLength;
		const std::uint16_t lastLightRegionY = regionToTest.getUpperCorner().getY() / mLightRegionSideLength;
		const std::uint16_t lastLightRegionZ = regionToTest.getUpperCorner().getZ() / mLightRegionSideLength;

		for(uint16_t zCt = firstLightRegionZ; zCt <= lastLightRegionZ; zCt++)
		{
			for(uint16_t yCt = firstLightRegionY; yCt <= lastLightRegionY; yCt++)
			{
				for(uint16_t xCt = firstLightRegionX; xCt <= lastLightRegionX; xCt++)
				{
					//volRegionLastModified->setVoxelAt(xCt,yCt,zCt,m_uCurrentTime);
					mLightLastModifiedArray[xCt][yCt][zCt] = globals.timeStamp();
				}
			}
		}
	}

	void Volume::createSphereAt(QVector3D centre, float radius, int material, bool bPaintMode)
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
						Material8 value(material);
						m_pPolyVoxVolume->setVoxelAt(x,y,z,value);
					}
				}
			}
		}

		updateLastModifedArray(regionToLock);

		qApp->getLogByName("Thermite")->logMessage("Volume was modified", QtOgre::LL_INFO);
	}

	void Volume::createCuboidAt(QVector3D centre, QVector3D dimensions, int material, bool bPaintMode)
	{
		dimensions /= 2.0f;

		int firstX = static_cast<int>(std::ceil(centre.x() - dimensions.x()));
		int firstY = static_cast<int>(std::ceil(centre.y() - dimensions.y()));
		int firstZ = static_cast<int>(std::ceil(centre.z() - dimensions.z()));

		int lastX = static_cast<int>(std::floor(centre.x() + dimensions.x()));
		int lastY = static_cast<int>(std::floor(centre.y() + dimensions.y()));
		int lastZ = static_cast<int>(std::floor(centre.z() + dimensions.z()));

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
					Material8 value(material);
					m_pPolyVoxVolume->setVoxelAt(x,y,z,value);
				}
			}
		}

		updateLastModifedArray(regionToLock);

		qApp->getLogByName("Thermite")->logMessage("Volume was modified", QtOgre::LL_INFO);
	}

	QVector3D Volume::getRayVolumeIntersection(QVector3D rayOrigin, const QVector3D& rayDir)
	{
		QVector3D result = QVector3D(0,0,0);

		Vector3DFloat start(rayOrigin.x(), rayOrigin.y(), rayOrigin.z());
		Vector3DFloat direction(rayDir.x(), rayDir.y(), rayDir.z());
		direction.normalise();
		direction *= 1000.0f;

		RaycastResult raycastResult;
		Raycast<Material8> raycast(m_pPolyVoxVolume, start, direction, raycastResult);
		raycast.execute();
		
		if(raycastResult.foundIntersection)
		{
			result = QVector3D(raycastResult.intersectionVoxel.getX(), raycastResult.intersectionVoxel.getY(), raycastResult.intersectionVoxel.getZ());
		}

		return result;
	}

	bool Volume::loadFromFile(const QString& filename)
	{
		uint16_t regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();
		PolyVox::Volume<PolyVox::Material8>* pPolyVoxVolume = VolumeManager::getSingletonPtr()->load(filename.toStdString(), "General")->getVolume();
		setPolyVoxVolume(pPolyVoxVolume, regionSideLength);
		return true;
	}

	void Volume::generateMapForTankWars(void)
	{
		const int mapWidth = 256;
		const int mapHeight = 32;
		const int mapDepth = 256;

		int volumeHeight = 64;

		PolyVox::Volume<PolyVox::Material8>* pPolyVoxVolume = new PolyVox::Volume<PolyVox::Material8>(mapWidth,volumeHeight,mapDepth);

		//Create a grid of Perlin noise values
		Perlin perlin(2,4,1,234);
		float perlinValues[mapWidth][mapDepth];
		float minPerlinValue = 1000.0f;
		float maxPerlinValue = -1000.0f;
		for(int z = 0; z < mapDepth; z++)
		{
			for(int x = 0; x < mapWidth; x++) 
			{
				perlinValues[x][z] = perlin.Get(x /static_cast<float>(mapWidth-1), z / static_cast<float>(mapDepth-1));
				minPerlinValue = std::min(minPerlinValue, perlinValues[x][z]);
				maxPerlinValue = std::max(maxPerlinValue, perlinValues[x][z]);
			}
		}

		//Normalise values so that th smallest is 0.0 and the biggest is 1.0
		float range = maxPerlinValue - minPerlinValue;
		for(int z = 0; z < mapDepth; z++)
		{
			for(int x = 0; x < mapWidth; x++) 
			{
				perlinValues[x][z] = (perlinValues[x][z] - minPerlinValue) / range;
			}
		}

		//Introduce a flat area into the map. This code saves the top and bottom parts and collapses the rest.
		for(int z = 0; z < mapDepth; z++)
		{
			for(int x = 0; x < mapWidth; x++) 
			{				
				float flatAreaSize = 1.0; //0.0 gives no flat area, larger number give increasing flat area.

				perlinValues[x][z] = perlinValues[x][z] * (flatAreaSize + 1.0f);

				float desiredGroundHeight = 0.25f;
				if(perlinValues[x][z] > desiredGroundHeight)
				{
					perlinValues[x][z] = std::max(desiredGroundHeight, perlinValues[x][z] - flatAreaSize);
				}				
			}
		}

		//Copy the data into the volume
		for(int z = 0; z < mapDepth; z++)
		{
			for(int x = 0; x < mapWidth; x++) 
			{							
				int terrainHeight = perlinValues[x][z] * (mapHeight-1);

				for(int y = 0; y < mapHeight; y++)
				{
					Material8 voxel;
					if(y < terrainHeight)
					{
						voxel.setMaterial(130);
						voxel.setDensity(Material8::getMaxDensity());
					}
					else if(y == terrainHeight)
					{
						voxel.setMaterial(60);
						voxel.setDensity(Material8::getMaxDensity());
					}
					else
					{
						voxel.setMaterial(0);
						voxel.setDensity(Material8::getMinDensity());
					}

					pPolyVoxVolume->setVoxelAt(x,y,z,voxel);
				}
			}
		}

		for(int z = 50; z < 150; z++)
		{
			for(int x = 50; x < 150; x++) 
			{
				Material8 voxel;
				voxel.setMaterial(130);
				voxel.setDensity(Material8::getMaxDensity());
				pPolyVoxVolume->setVoxelAt(x,15,z,voxel);
			}
		}

		uint16_t regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();
		setPolyVoxVolume(pPolyVoxVolume, regionSideLength);
		return;
	}

	int Volume::materialAtPosition(QVector3D position)
	{
		int x = qRound(position.x());
		int y = qRound(position.y());
		int z = qRound(position.z());

		Material8 voxel = m_pPolyVoxVolume->getVoxelAt(x,y,z);

		return voxel.getMaterial();
	}

	void Volume::findPath(QVector3D start, QVector3D end)
	{
		FindPathTask* findPathTask = new FindPathTask(m_pPolyVoxVolume, start, end, this);
		connect(findPathTask, SIGNAL(finished(QVariantList)), this, SLOT(finishedHandler(QVariantList)));
		
		//findPathTask->run();
		QThreadPool::globalInstance()->start(findPathTask, 1);
	}

	void Volume::finishedHandler(QVariantList path)
	{
		emit foundPath(path);
	}
}
