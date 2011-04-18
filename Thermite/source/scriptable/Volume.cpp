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

#include "VolumeManager.h"

#include "Material.h"

#include "Raycast.h"
#include "AmbientOcclusionCalculator.h"

#include "Utility.h"

#include "FindPathTask.h"
#include "AmbientOcclusionTask.h"
#include "SurfaceMeshExtractionTask.h"
#include "SurfaceMeshDecimationTask.h"
#include "SurfacePatchRenderable.h"
#include "TaskProcessorThread.h"

#include "OgreSceneManager.h"
#include "OgreSceneNode.h"

#include <QSettings>
#include <QThreadPool>

using namespace PolyVox;

namespace Thermite
{
	TaskProcessorThread* Volume::m_backgroundThread = 0;

	Volume::Volume(uint32_t width, uint32_t height, uint32_t depth, Object* parent)
		:QObject(parent)
		,m_pPolyVoxVolume(0)
		,mVolumeWidthInRegions(0)
		,mVolumeHeightInRegions(0)
		,mVolumeDepthInRegions(0)
		,mMultiThreadedSurfaceExtraction(false)
		,mCachedVolumeWidthInRegions(0)
		,mCachedVolumeHeightInRegions(0)
		,mCachedVolumeDepthInRegions(0)

		,mCachedVolumeWidthInLightRegions(0)
		,mCachedVolumeHeightInLightRegions(0)
		,mCachedVolumeDepthInLightRegions(0)

		,mVolumeSceneNode(0)
		,mIsModified(true)
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

		uint16_t regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 32).toInt();
		PolyVox::Volume<PolyVox::Material8>* pPolyVoxVolume = new PolyVox::Volume<PolyVox::Material8>(Region(Vector3DInt32(0,0,0), Vector3DInt32(width-1, height-1, depth-1)));
		pPolyVoxVolume->setCompressionEnabled(false);
		setPolyVoxVolume(pPolyVoxVolume, regionSideLength);
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

		//Ambient Occlusion
		mAmbientOcclusionVolume.resize(ArraySizes(32)(8)(32));
		std::fill(mAmbientOcclusionVolume.getRawData(), mAmbientOcclusionVolume.getRawData() + mAmbientOcclusionVolume.getNoOfElements(), 0);

		/*QTime time;
		time.start();
		AmbientOcclusionCalculator<Material8> ambientOcclusionCalculator(m_pPolyVoxVolume, &mAmbientOcclusionVolume, m_pPolyVoxVolume->getEnclosingRegion(), mLightRegionSideLength);
		ambientOcclusionCalculator.execute();
		qDebug() << "Lighting time = " << time.elapsed();*/
	}

	void Volume::initialise(void)
	{		
	}

	void Volume::update(void)
	{
		//Update the Volume properties
		if(mIsModified)
		{	
			Ogre::SceneManager* sceneManager = Ogre::Root::getSingletonPtr()->getSceneManager("OgreSceneManager");

			//Create a scene node to attach this volume under
			if(mVolumeSceneNode == 0)
			{
				mVolumeSceneNode =  sceneManager->getRootSceneNode()->createChildSceneNode("VolumeSceneNode");
			}

			//If the size of the volume has changed then we need to start from scratch by throwing away our data and regenerating.
			if((mCachedVolumeWidthInRegions != mVolumeWidthInRegions) || (mCachedVolumeHeightInRegions != mVolumeHeightInRegions) || (mCachedVolumeDepthInRegions != mVolumeDepthInRegions))
			{	
				deleteSceneNodeChildren(mVolumeSceneNode);

				mCachedVolumeWidthInRegions = mVolumeWidthInRegions;
				mCachedVolumeHeightInRegions = mVolumeHeightInRegions;
				mCachedVolumeDepthInRegions = mVolumeDepthInRegions;

				//m_axisNode->setScale(m_pPolyVoxVolume->getWidth(), m_pPolyVoxVolume->getHeight(), m_pPolyVoxVolume->getDepth());
						
				uint32_t dimensions[3] = {mCachedVolumeWidthInRegions, mCachedVolumeHeightInRegions, mCachedVolumeDepthInRegions}; // Array dimensions

				//Create the arrays
				mVolLastUploadedTimeStamps.resize(dimensions);
				m_volOgreSceneNodes.resize(dimensions);

				//Lighting
				mCachedVolumeWidthInLightRegions = mVolumeWidthInLightRegions;
				mCachedVolumeHeightInLightRegions = mVolumeHeightInLightRegions;
				mCachedVolumeDepthInLightRegions = mVolumeDepthInLightRegions;
						
				uint32_t lightDimensions[3] = {mCachedVolumeWidthInLightRegions, mCachedVolumeHeightInLightRegions, mCachedVolumeDepthInLightRegions}; // Array dimensions

				//Create the arrays
				mVolLightingLastUploadedTimeStamps.resize(lightDimensions);

				//Clear the arrays
				std::fill(mVolLastUploadedTimeStamps.getRawData(), mVolLastUploadedTimeStamps.getRawData() + mVolLastUploadedTimeStamps.getNoOfElements(), 0);						
				std::fill(m_volOgreSceneNodes.getRawData(), m_volOgreSceneNodes.getRawData() + m_volOgreSceneNodes.getNoOfElements(), (Ogre::SceneNode*)0);

				std::fill(mVolLightingLastUploadedTimeStamps.getRawData(), mVolLightingLastUploadedTimeStamps.getRawData() + mVolLightingLastUploadedTimeStamps.getNoOfElements(), 0);

				//Resize the ambient occlusion volume texture
				if(mAmbientOcclusionVolumeTexture.isNull() == false)
				{
					//Not sure if we actually need to (or even should) remove the old one first - maybe the smart pointer handles it.
					Ogre::TextureManager::getSingleton().remove("AmbientOcclusionVolumeTexture");
				}

				const int iRatio = 4; //Ratio of ambient occlusion volume size to main volume size.
				mAmbientOcclusionVolumeTexture = Ogre::TextureManager::getSingleton().createManual(
					"AmbientOcclusionVolumeTexture", // Name of texture
					"General", // Name of resource group in which the texture should be created
					Ogre::TEX_TYPE_3D, // Texture type
					m_pPolyVoxVolume->getWidth() / iRatio, // Width
					m_pPolyVoxVolume->getHeight() / iRatio, // Height
					m_pPolyVoxVolume->getDepth() / iRatio, // Depth (Must be 1 for two dimensional textures)
					0, // Number of mipmaps
					Ogre::PF_L8, // Pixel format
					Ogre::TU_STATIC_WRITE_ONLY // usage
					);

				Ogre::MaterialPtr materialPtr = Ogre::MaterialManager::getSingleton().getByName("ColouredCubicVoxel");
				Ogre::Technique* pTechnique = materialPtr->getTechnique("Advanced");
				Ogre::Pass* pPass = pTechnique->getPass("Light0");
				Ogre::TextureUnitState* pTexUnit = pPass->createTextureUnitState();
				pTexUnit->setTextureName("AmbientOcclusionVolumeTexture", Ogre::TEX_TYPE_3D);

				materialPtr = Ogre::MaterialManager::getSingleton().getByName("VertexColourMaterial");
				pTechnique = materialPtr->getTechnique("Advanced");
				pPass = pTechnique->getPass("Light0");
				pTexUnit = pPass->createTextureUnitState();
				pTexUnit->setTextureName("AmbientOcclusionVolumeTexture", Ogre::TEX_TYPE_3D);
				pTexUnit->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
			}

			//Some values we'll need later.
			uint16_t volumeWidthInRegions = mVolumeWidthInRegions;
			uint16_t volumeHeightInRegions = mVolumeHeightInRegions;
			uint16_t volumeDepthInRegions = mVolumeDepthInRegions;

			//Iterate over each region
			for(std::uint16_t regionZ = 0; regionZ < volumeDepthInRegions; ++regionZ)
			{		
				for(std::uint16_t regionY = 0; regionY < volumeHeightInRegions; ++regionY)
				{
					for(std::uint16_t regionX = 0; regionX < volumeWidthInRegions; ++regionX)
					{
						uint32_t volExtractionFinsishedTimeStamp = mExtractionFinishedArray[regionX][regionY][regionZ];
						uint32_t volLastUploadedTimeStamp = mVolLastUploadedTimeStamps[regionX][regionY][regionZ];
						if(volExtractionFinsishedTimeStamp > volLastUploadedTimeStamp)
						{
							SurfaceMesh<PositionMaterial>* mesh = m_volSurfaceMeshes[regionX][regionY][regionZ];
							PolyVox::Region reg = mesh->m_Region;
							uploadSurfaceMesh(*(m_volSurfaceMeshes[regionX][regionY][regionZ]), reg, *this);
						}
					}
				}
			}

			//Some values we'll need later.
			uint16_t volumeWidthInLightRegions = mVolumeWidthInLightRegions;
			uint16_t volumeHeightInLightRegions = mVolumeHeightInLightRegions;
			uint16_t volumeDepthInLightRegions = mVolumeDepthInLightRegions;

			bool needsLightingUpload = false;
			//Iterate over each region
			for(std::uint16_t regionZ = 0; regionZ < volumeDepthInLightRegions; ++regionZ)
			{		
				for(std::uint16_t regionY = 0; regionY < volumeHeightInLightRegions; ++regionY)
				{
					for(std::uint16_t regionX = 0; regionX < volumeWidthInLightRegions; ++regionX)
					{
						uint32_t volLightingFinsishedTimeStamp = mLightingFinishedArray[regionX][regionY][regionZ];
						uint32_t volLightingLastUploadedTimeStamp = mVolLightingLastUploadedTimeStamps[regionX][regionY][regionZ];
						if(volLightingFinsishedTimeStamp > volLightingLastUploadedTimeStamp)
						{
							needsLightingUpload = true;

							mVolLightingLastUploadedTimeStamps[regionX][regionY][regionZ] = globals.timeStamp();
						}
					}
				}
			}
			//Ambient Occlusion
			if(needsLightingUpload)
			{
				Ogre::HardwarePixelBuffer* pixelBuffer = mAmbientOcclusionVolumeTexture.getPointer()->getBuffer().getPointer();
				Ogre::PixelBox pixelBox(mAmbientOcclusionVolumeTexture->getWidth(),mAmbientOcclusionVolumeTexture->getHeight(),mAmbientOcclusionVolumeTexture->getDepth(), mAmbientOcclusionVolumeTexture->getFormat(), mAmbientOcclusionVolume.getRawData());
				pixelBuffer->blitFromMemory(pixelBox);
			}
		}
		mIsModified = false;
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
							Vector3DInt32 v3dLowerCorner(firstX,firstY,firstZ);
							Vector3DInt32 v3dUpperCorner(lastX,lastY,lastZ);
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

							Vector3DInt32 v3dLowerCorner(firstX,firstY,firstZ);
							Vector3DInt32 v3dUpperCorner(lastX,lastY,lastZ);
							PolyVox::Region region(v3dLowerCorner, v3dUpperCorner);
							region.cropTo(m_pPolyVoxVolume->getEnclosingRegion());

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

			mIsModified = true;
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
		firstX = std::max(firstX,m_pPolyVoxVolume->getEnclosingRegion().getLowerCorner().getX());
		firstY = std::max(firstY,m_pPolyVoxVolume->getEnclosingRegion().getLowerCorner().getY());
		firstZ = std::max(firstZ,m_pPolyVoxVolume->getEnclosingRegion().getLowerCorner().getZ());

		lastX = std::min(lastX,m_pPolyVoxVolume->getEnclosingRegion().getUpperCorner().getX());
		lastY = std::min(lastY,m_pPolyVoxVolume->getEnclosingRegion().getUpperCorner().getY());
		lastZ = std::min(lastZ,m_pPolyVoxVolume->getEnclosingRegion().getUpperCorner().getZ());

		PolyVox::Region regionToLock = PolyVox::Region(PolyVox::Vector3DInt32(firstX, firstY, firstZ), PolyVox::Vector3DInt32(lastX, lastY, lastZ));

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

		PolyVox::Region regionToLock = PolyVox::Region(PolyVox::Vector3DInt32(firstX, firstY, firstZ), PolyVox::Vector3DInt32(lastX, lastY, lastZ));

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
	}

	QPair<bool, QVector3D> Volume::getRayVolumeIntersection(QVector3D rayOrigin, const QVector3D& rayDir)
	{
		QPair<bool, QVector3D> result;
		result.first = false;
		result.second = QVector3D(0,0,0);

		Vector3DFloat start(rayOrigin.x(), rayOrigin.y(), rayOrigin.z());
		Vector3DFloat direction(rayDir.x(), rayDir.y(), rayDir.z());
		direction.normalise();
		direction *= 1000.0f;

		RaycastResult raycastResult;
		Raycast<Material8> raycast(m_pPolyVoxVolume, start, direction, raycastResult);
		raycast.execute();
		
		if(raycastResult.foundIntersection)
		{
			result.first = true;
			result.second = QVector3D(raycastResult.intersectionVoxel.getX(), raycastResult.intersectionVoxel.getY(), raycastResult.intersectionVoxel.getZ());
		}

		return result;
	}

	bool Volume::loadFromFile(const QString& filename)
	{
		uint16_t regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 32).toInt();
		PolyVox::Volume<PolyVox::Material8>* pPolyVoxVolume = VolumeManager::getSingletonPtr()->load(filename.toStdString(), "General")->getVolume();
		setPolyVoxVolume(pPolyVoxVolume, regionSideLength);
		return true;
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

	void Volume::uploadSurfaceMesh(const SurfaceMesh<PositionMaterial>& mesh, PolyVox::Region region, Volume& volume)
	{
		bool bSimulatePhysics = qApp->settings()->value("Physics/SimulatePhysics", false).toBool();

		//Determine where it came from
		uint16_t regionX = region.getLowerCorner().getX() / volume.mRegionSideLength;
		uint16_t regionY = region.getLowerCorner().getY() / volume.mRegionSideLength;
		uint16_t regionZ = region.getLowerCorner().getZ() / volume.mRegionSideLength;

		//Create a SceneNode for that location if we don't have one already
		Ogre::SceneNode* pOgreSceneNode = m_volOgreSceneNodes[regionX][regionY][regionZ];
		if(pOgreSceneNode == 0)
		{
			const std::string& strNodeName = generateUID("SN");
			pOgreSceneNode = mVolumeSceneNode->createChildSceneNode(strNodeName);
			pOgreSceneNode->setPosition(Ogre::Vector3(region.getLowerCorner().getX(),region.getLowerCorner().getY(),region.getLowerCorner().getZ()));
			m_volOgreSceneNodes[regionX][regionY][regionZ] = pOgreSceneNode;
		}
		else
		{
			deleteSceneNodeChildren(pOgreSceneNode);
		}

		//Get the SurfaceMesh and check it's valid
		SurfaceMesh<PositionMaterial> meshWhole = mesh;
		if(meshWhole.isEmpty() == false)
		{			
			addSurfacePatchRenderable(volume.m_mapMaterialIds.begin()->first, meshWhole, region); ///[0] is HACK!!

			//The SurfaceMesh needs to be broken into pieces - one for each material. Iterate over the materials...
			/*for(std::map< std::string, std::set<uint8_t> >::iterator iter = volume.m_mapMaterialIds.begin(); iter != volume.m_mapMaterialIds.end(); iter++)
			{
				//Get the properties
				std::string materialName = iter->first;
				std::set<std::uint8_t> voxelValues = iter->second;

				//Extract the part of the InexedSurfacePatch which corresponds to that material
				polyvox_shared_ptr< SurfaceMesh<PositionMaterialNormal> > meshSubset = meshWhole.extractSubset(voxelValues);

				//And add it to the SceneNode
				addSurfacePatchRenderable(materialName, *meshSubset, region);
			}*/
		}		

		mVolLastUploadedTimeStamps[regionX][regionY][regionZ] = globals.timeStamp();
	}

	void Volume::addSurfacePatchRenderable(std::string materialName, SurfaceMesh<PositionMaterial>& mesh, PolyVox::Region region)
	{

		std::uint16_t regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 32).toInt();

		//Determine where it came from
		uint16_t regionX = region.getLowerCorner().getX() / regionSideLength;
		uint16_t regionY = region.getLowerCorner().getY() / regionSideLength;
		uint16_t regionZ = region.getLowerCorner().getZ() / regionSideLength;

		Ogre::SceneNode* pOgreSceneNode = m_volOgreSceneNodes[regionX][regionY][regionZ];

		//Single Material
		SurfacePatchRenderable* pSingleMaterialSurfacePatchRenderable;

		std::string strSprName = generateUID("SPR");
		Ogre::SceneManager* sceneManager = Ogre::Root::getSingletonPtr()->getSceneManager("OgreSceneManager");
		pSingleMaterialSurfacePatchRenderable = dynamic_cast<SurfacePatchRenderable*>(sceneManager->createMovableObject(strSprName, SurfacePatchRenderableFactory::FACTORY_TYPE_NAME));
		pSingleMaterialSurfacePatchRenderable->setMaterial(materialName);
		pSingleMaterialSurfacePatchRenderable->setCastShadows(true);
		pOgreSceneNode->attachObject(pSingleMaterialSurfacePatchRenderable);
		pSingleMaterialSurfacePatchRenderable->m_v3dPos = pOgreSceneNode->getPosition();

		pSingleMaterialSurfacePatchRenderable->buildRenderOperationFrom(mesh);

		Ogre::AxisAlignedBox aabb(Ogre::Vector3(0.0f,0.0f,0.0f), Ogre::Vector3(regionSideLength, regionSideLength, regionSideLength));
		pSingleMaterialSurfacePatchRenderable->setBoundingBox(aabb);
	}

	void Volume::uploadSurfaceMesh(const SurfaceMesh<PositionMaterialNormal>& mesh, PolyVox::Region region, Volume& volume)
	{
		bool bSimulatePhysics = qApp->settings()->value("Physics/SimulatePhysics", false).toBool();

		//Determine where it came from
		uint16_t regionX = region.getLowerCorner().getX() / volume.mRegionSideLength;
		uint16_t regionY = region.getLowerCorner().getY() / volume.mRegionSideLength;
		uint16_t regionZ = region.getLowerCorner().getZ() / volume.mRegionSideLength;

		//Create a SceneNode for that location if we don't have one already
		Ogre::SceneNode* pOgreSceneNode = m_volOgreSceneNodes[regionX][regionY][regionZ];
		if(pOgreSceneNode == 0)
		{
			const std::string& strNodeName = generateUID("SN");
			pOgreSceneNode = mVolumeSceneNode->createChildSceneNode(strNodeName);
			pOgreSceneNode->setPosition(Ogre::Vector3(region.getLowerCorner().getX(),region.getLowerCorner().getY(),region.getLowerCorner().getZ()));
			m_volOgreSceneNodes[regionX][regionY][regionZ] = pOgreSceneNode;
		}
		else
		{
			deleteSceneNodeChildren(pOgreSceneNode);
		}

		//Clear any previous geometry		
		/*Ogre::SceneNode::ObjectIterator iter =  pOgreSceneNode->getAttachedObjectIterator();
		while (iter.hasMoreElements())
		{
			Ogre::MovableObject* obj = iter.getNext();
			mOgreSceneManager->destroyMovableObject(obj);
		}
		pOgreSceneNode->detachAllObjects();*/

		//Get the SurfaceMesh and check it's valid
		SurfaceMesh<PositionMaterialNormal> meshWhole = mesh;
		if(meshWhole.isEmpty() == false)
		{			
			//The SurfaceMesh needs to be broken into pieces - one for each material. Iterate over the materials...
			for(std::map< std::string, std::set<uint8_t> >::iterator iter = volume.m_mapMaterialIds.begin(); iter != volume.m_mapMaterialIds.end(); iter++)
			{
				//Get the properties
				std::string materialName = iter->first;
				std::set<std::uint8_t> voxelValues = iter->second;

				//Extract the part of the InexedSurfacePatch which corresponds to that material
				//polyvox_shared_ptr< SurfaceMesh<PositionMaterialNormal> > meshSubset = meshWhole.extractSubset(voxelValues);
				polyvox_shared_ptr< SurfaceMesh<PositionMaterialNormal> > meshSubset = extractSubset(meshWhole, voxelValues);

				//And add it to the SceneNode
				addSurfacePatchRenderable(materialName, meshWhole, region);
			}
		}		

		mVolLastUploadedTimeStamps[regionX][regionY][regionZ] = globals.timeStamp();
	}

	void Volume::addSurfacePatchRenderable(std::string materialName, SurfaceMesh<PositionMaterialNormal>& mesh, PolyVox::Region region)
	{

		std::uint16_t regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 32).toInt();

		//Determine where it came from
		uint16_t regionX = region.getLowerCorner().getX() / regionSideLength;
		uint16_t regionY = region.getLowerCorner().getY() / regionSideLength;
		uint16_t regionZ = region.getLowerCorner().getZ() / regionSideLength;

		Ogre::SceneNode* pOgreSceneNode = m_volOgreSceneNodes[regionX][regionY][regionZ];

		//Single Material
		SurfacePatchRenderable* pSingleMaterialSurfacePatchRenderable;

		std::string strSprName = generateUID("SPR");
		Ogre::SceneManager* sceneManager = Ogre::Root::getSingletonPtr()->getSceneManager("OgreSceneManager");
		pSingleMaterialSurfacePatchRenderable = dynamic_cast<SurfacePatchRenderable*>(sceneManager->createMovableObject(strSprName, SurfacePatchRenderableFactory::FACTORY_TYPE_NAME));
		pSingleMaterialSurfacePatchRenderable->setMaterial(materialName);
		pSingleMaterialSurfacePatchRenderable->setCastShadows(true);
		pOgreSceneNode->attachObject(pSingleMaterialSurfacePatchRenderable);
		pSingleMaterialSurfacePatchRenderable->m_v3dPos = pOgreSceneNode->getPosition();

		pSingleMaterialSurfacePatchRenderable->buildRenderOperationFrom(mesh, true);

		Ogre::AxisAlignedBox aabb(Ogre::Vector3(0.0f,0.0f,0.0f), Ogre::Vector3(regionSideLength, regionSideLength, regionSideLength));
		pSingleMaterialSurfacePatchRenderable->setBoundingBox(aabb);

		//Multi material
		SurfacePatchRenderable* pMultiMaterialSurfacePatchRenderable;

		//Create additive material
		Ogre::String strAdditiveMaterialName = materialName + "_WITH_ADDITIVE_BLENDING";
		Ogre::MaterialPtr additiveMaterial = Ogre::MaterialManager::getSingleton().getByName(strAdditiveMaterialName);
		if(additiveMaterial.isNull() == true)
		{
			Ogre::MaterialPtr originalMaterial = Ogre::MaterialManager::getSingleton().getByName(materialName);
			additiveMaterial = originalMaterial->clone(strAdditiveMaterialName);
			additiveMaterial->setSceneBlending(Ogre::SBT_ADD);
		}

		strSprName = generateUID("SPR");
		pMultiMaterialSurfacePatchRenderable = dynamic_cast<SurfacePatchRenderable*>(sceneManager->createMovableObject(strSprName, SurfacePatchRenderableFactory::FACTORY_TYPE_NAME));
		pMultiMaterialSurfacePatchRenderable->setMaterial(strAdditiveMaterialName);
		pMultiMaterialSurfacePatchRenderable->setCastShadows(true);
		pOgreSceneNode->attachObject(pMultiMaterialSurfacePatchRenderable);
		pMultiMaterialSurfacePatchRenderable->m_v3dPos = pOgreSceneNode->getPosition();

		pMultiMaterialSurfacePatchRenderable->buildRenderOperationFrom(mesh, false);

		pMultiMaterialSurfacePatchRenderable->setBoundingBox(aabb);
	}

	void Volume::deleteSceneNodeChildren(Ogre::SceneNode* sceneNode)
	{
		//Delete any attached objects
		Ogre::SceneNode::ObjectIterator iter =  sceneNode->getAttachedObjectIterator();
		while (iter.hasMoreElements())
		{
			//Destroy the objects (leaves dangling pointers?)
			Ogre::MovableObject* obj = iter.getNext();
			Ogre::SceneManager* sceneManager = Ogre::Root::getSingletonPtr()->getSceneManager("OgreSceneManager");
			sceneManager->destroyMovableObject(obj);
		}
		//Clean up all dangling pointers.
		sceneNode->detachAllObjects();

		//Delete any child nodes
		Ogre::Node::ChildNodeIterator childNodeIter = sceneNode->getChildIterator();
		while(childNodeIter.hasMoreElements())
		{
			//A Node has to actually be a SceneNode or a Bone. We are not concerned with Bones at the moment.
			Ogre::SceneNode* childSceneNode = dynamic_cast<Ogre::SceneNode*>(childNodeIter.getNext());
			if(childSceneNode)
			{
				//Recursive call
				deleteSceneNodeChildren(childSceneNode);
			}
		}		
	}
}
