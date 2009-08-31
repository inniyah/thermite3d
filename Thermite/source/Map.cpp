#pragma region License
/******************************************************************************
This file is part of the Thermite 3D game engine
Copyright (C) 2006  David Williams

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
******************************************************************************/
#pragma endregion

#include "Map.h"

#include "Application.h"
#include "MapHandler.h"
#include "VolumeManager.h"

#include "SurfaceExtractorThread.h"

#include "VolumeChangeTracker.h"
#include "SurfaceExtractor.h"

#include "SurfacePatchRenderable.h"
#include "MapRegion.h"

#include "OgreBulletDynamicsWorld.h"

#include "Shapes/OgreBulletCollisionsBoxShape.h"

#include "ThermiteGameLogic.h"

#include <OgreSceneManagerEnumerator.h>
#include <OgreSceneManager.h>

#include <QSettings>

//using namespace Ogre;
using namespace PolyVox;
using namespace OgreBulletDynamics;
using namespace OgreBulletCollisions;
using PolyVox::uint32_t;
using PolyVox::uint16_t;
using PolyVox::uint8_t;

namespace Thermite
{
	Map::Map(Ogre::Vector3 vecGravity, Ogre::AxisAlignedBox boxPhysicsBounds, Ogre::Real rVoxelSize, Ogre::SceneManager* sceneManager)
		:m_vecGravity(vecGravity)
		,m_boxPhysicsBounds(boxPhysicsBounds)
		,m_rVoxelSize(rVoxelSize)
		,m_volMapRegions(0)
	{
		//memset(m_iRegionTimeStamps, 0xFF, sizeof(m_iRegionTimeStamps));

		m_pOgreSceneManager = sceneManager;

		initialisePhysics();
	}

	Map::~Map(void)
	{

	}

	void Map::initialisePhysics(void)
	{
		const Ogre::Vector3 gravityVector = Ogre::Vector3 (0,-98.1,0);
		const Ogre::AxisAlignedBox bounds = Ogre::AxisAlignedBox (Ogre::Vector3 (-10000, -10000, -10000),Ogre::Vector3 (10000,  10000,  10000));
		m_pOgreBulletWorld = new DynamicsWorld(m_pOgreSceneManager, bounds, gravityVector);
	}

	bool Map::loadScene(const Ogre::String& filename)
	{
		m_iNoProcessed = 0;
		m_iNoSubmitted = 0;

		MapHandler handler(this);
		QXmlSimpleReader reader;
		reader.setContentHandler(&handler);
		reader.setErrorHandler(&handler);

		//QFile file("..\\share\\thermite\\Ogre\\maps\\" + QString::fromStdString(filename)); //HACK - Should really use the resource system for this!
		//file.open(QFile::ReadOnly | QFile::Text);
		QXmlInputSource xmlInputSource;
		xmlInputSource.setData(QString::fromStdString(filename));
		reader.parse(xmlInputSource);

		//This gets the first camera which was found in the scene.
		Ogre::SceneManager::CameraIterator camIter = m_pOgreSceneManager->getCameraIterator();
		m_pCamera = camIter.peekNextValue();

		int regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();
		int volumeWidthInRegions = volumeChangeTracker->getWrappedVolume()->getWidth() / regionSideLength;
		int volumeHeightInRegions = volumeChangeTracker->getWrappedVolume()->getHeight() / regionSideLength;
		int volumeDepthInRegions = volumeChangeTracker->getWrappedVolume()->getDepth() / regionSideLength;

		m_volRegionTimeStamps = new Volume<uint32_t>(volumeWidthInRegions, volumeHeightInRegions, volumeDepthInRegions, 0);
		m_volMapRegions = new Volume<MapRegion*>(volumeWidthInRegions, volumeHeightInRegions, volumeDepthInRegions, 0);

		volumeChangeTracker->setAllRegionsModified();

		m_pMTSE = new MultiThreadedSurfaceExtractor(volumeChangeTracker->getWrappedVolume(), qApp->settings()->value("Engine/NoOfSurfaceExtractionThreads", 2).toInt());
		m_pMTSE->start();

		return true;
	}

	void Map::updatePolyVoxGeometry()
	{
		if(!volumeResource.isNull())
		{		
			//Some values we'll need later.
			uint16_t regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();
			uint16_t halfRegionSideLength = regionSideLength / 2;
			uint16_t volumeWidthInRegions = volumeResource->getVolume()->getWidth() / regionSideLength;
			uint16_t volumeHeightInRegions = volumeResource->getVolume()->getHeight() / regionSideLength;
			uint16_t volumeDepthInRegions = volumeResource->getVolume()->getDepth() / regionSideLength;

			double fLod0ToLod1Boundary = qApp->settings()->value("Engine/Lod0ToLod1Boundary", 256.0f).toDouble();
			double fLod0ToLod1BoundarySquared = fLod0ToLod1Boundary * fLod0ToLod1Boundary;
			double fLod1ToLod2Boundary = qApp->settings()->value("Engine/Lod1ToLod2Boundary", 512.0f).toDouble();
			double fLod1ToLod2BoundarySquared = fLod1ToLod2Boundary * fLod1ToLod2Boundary;

			//Iterate over each region in the VolumeChangeTracker
			for(uint16_t regionZ = 0; regionZ < volumeDepthInRegions; ++regionZ)
			{		
				for(uint16_t regionY = 0; regionY < volumeHeightInRegions; ++regionY)
				{
					for(uint16_t regionX = 0; regionX < volumeWidthInRegions; ++regionX)
					{
						//Compute the extents of the current region
						const uint16_t firstX = regionX * regionSideLength;
						const uint16_t firstY = regionY * regionSideLength;
						const uint16_t firstZ = regionZ * regionSideLength;

						const uint16_t lastX = firstX + regionSideLength;
						const uint16_t lastY = firstY + regionSideLength;
						const uint16_t lastZ = firstZ + regionSideLength;	

						const float centreX = firstX + halfRegionSideLength;
						const float centreY = firstY + halfRegionSideLength;
						const float centreZ = firstZ + halfRegionSideLength;

						//The regions distance from the camera is used for
						//LOD selection and prioritizing surface extraction
						Ogre::Vector3 cameraPos = m_pCamera->getPosition();
						Ogre::Vector3 centre(centreX, centreY, centreZ);
						double distanceFromCameraSquared = (cameraPos - centre).squaredLength();

						//There's no guarentee that the MapRegion exists at this point...
						MapRegion* pMapRegion = m_volMapRegions->getVoxelAt(regionX, regionY, regionZ);
						if(pMapRegion)
						{							
							//But if it does, we set the appropriate LOD level based on distance from the camera.
							if((distanceFromCameraSquared > fLod1ToLod2BoundarySquared) && (fLod1ToLod2Boundary > 0.0f))
							{
								pMapRegion->setLodLevelToUse(2);
							}
							else if((distanceFromCameraSquared > fLod0ToLod1BoundarySquared) && (fLod0ToLod1Boundary > 0.0f))
							{
								pMapRegion->setLodLevelToUse(1);
							}
							else
							{
								pMapRegion->setLodLevelToUse(0);
							}
						}

						//If the region has changed then we may need to add or remove MapRegion to/from the scene graph
						if(volumeChangeTracker->getLastModifiedTimeForRegion(regionX, regionY, regionZ) > m_volRegionTimeStamps->getVoxelAt(regionX,regionY,regionZ))
						{
							//Convert to a real PolyVox::Region
							Vector3DInt16 v3dLowerCorner(firstX,firstY,firstZ);
							Vector3DInt16 v3dUpperCorner(lastX,lastY,lastZ);
							PolyVox::Region region(v3dLowerCorner, v3dUpperCorner);
							region.cropTo(volumeChangeTracker->getWrappedVolume()->getEnclosingRegion());

							//The prioirty ensures that the surfaces for regions close to the
							//camera getextracted before those which are distant from the camera.
							uint32_t uPriority = std::numeric_limits<uint32_t>::max() - static_cast<uint32_t>(distanceFromCameraSquared);

							//Extract the region with a LOD level of 0
							m_pMTSE->pushTask(SurfaceExtractorTaskData(region, 0, uPriority));
							m_iNoSubmitted++;

							if(fLod0ToLod1Boundary > 0) //If the first LOD level is enabled
							{
								//Extract the region with a LOD level of 1
								m_pMTSE->pushTask(SurfaceExtractorTaskData(region, 1, uPriority));
								m_iNoSubmitted++;
							}

							if(fLod1ToLod2Boundary > 0) //If the second LOD level is enabled
							{
								//Extract the region with a LOD level of 2
								m_pMTSE->pushTask(SurfaceExtractorTaskData(region, 2, uPriority));
								m_iNoSubmitted++;
							}

							//Indicate that we've processed this region
							m_volRegionTimeStamps->setVoxelAt(regionX,regionY,regionZ,volumeChangeTracker->getLastModifiedTimeForRegion(regionX, regionY, regionZ));
						}
					}
				}
			}

			bool bSimulatePhysics = qApp->settings()->value("Physics/SimulatePhysics", false).toBool();
			int iPhysicsLOD = qApp->settings()->value("Physics/PhysicsLOD", 0).toInt();

			//Process any results which have been returned by the surface extractor threads.
			while(m_pMTSE->noOfResultsAvailable() > 0)
			{
				//Get the next available result
				SurfaceExtractorTaskData result;
				result = m_pMTSE->popResult();

				//Determine where it came from
				PolyVox::uint16_t regionX = result.getRegion().getLowerCorner().getX() / regionSideLength;
				PolyVox::uint16_t regionY = result.getRegion().getLowerCorner().getY() / regionSideLength;
				PolyVox::uint16_t regionZ = result.getRegion().getLowerCorner().getZ() / regionSideLength;

				//Create a MapRegion for that location if we don't have one already
				MapRegion* pMapRegion = m_volMapRegions->getVoxelAt(regionX, regionY, regionZ);
				if(pMapRegion == 0)
				{
					pMapRegion = new MapRegion(this, result.getRegion().getLowerCorner());
					m_volMapRegions->setVoxelAt(regionX, regionY, regionZ, pMapRegion);
				}

				//Get the IndexedSurfacePatch and check it's valid
				POLYVOX_SHARED_PTR<IndexedSurfacePatch> ispWhole = result.getIndexedSurfacePatch();
				if((ispWhole) && (ispWhole->isEmpty() == false))
				{
					//Clear any previous geometry
					pMapRegion->removeAllSurfacePatchRenderablesForLod(result.getLodLevel());

					//The IndexedSurfacePatch needs to be broken into pieces - one for each material. Iterate over the mateials...
					for(std::map< std::string, std::set<PolyVox::uint8_t> >::iterator iter = m_mapMaterialIds.begin(); iter != m_mapMaterialIds.end(); iter++)
					{
						//Get the properties
						std::string materialName = iter->first;
						std::set<uint8_t> voxelValues = iter->second;

						//Extract the part of the InexedSurfacePatch which corresponds to that material
						POLYVOX_SHARED_PTR<IndexedSurfacePatch> ispSubset = ispWhole->extractSubset(voxelValues);

						//And add it to the MapRegion
						pMapRegion->addSurfacePatchRenderable(materialName, *ispSubset, result.getLodLevel());
					}

					//If we are simulating physics and the current LOD matches...
					if((bSimulatePhysics) && (result.getLodLevel() == iPhysicsLOD))
					{
						//Update the physics geometry
						pMapRegion->setPhysicsData(*(ispWhole.get()));
					}
				}

				//Update the progress bar
				m_iNoProcessed++;
				float fProgress = static_cast<float>(m_iNoProcessed) / static_cast<float>(m_iNoSubmitted);
				m_pThermiteGameLogic->m_loadingProgress->setExtractingSurfacePercentageDone(fProgress*100);

				//If we've finished, the progress bar can be hidden.
				if(fProgress > 0.999)
				{
					m_pThermiteGameLogic->m_loadingProgress->hide();
					m_pThermiteGameLogic->bLoadComplete = true;
				}
			}
		}		
	}
}