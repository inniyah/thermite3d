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

#include "GradientEstimators.h"


#include <OgreSceneManagerEnumerator.h>
#include <OgreSceneManager.h>
#include <OgreTimer.h>

#include <QSettings>

//using namespace Ogre;
using namespace PolyVox;
using namespace OgreBulletDynamics;
using namespace OgreBulletCollisions;

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

		timer = new Ogre::Timer();

		initialisePhysics();

		m_pOgreSceneManager->setSkyBox(true, "SkyBox");
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

		QFile file("..\\share\\thermite\\Ogre\\maps\\" + QString::fromStdString(filename)); //HACK - Should really use the resource system for this!
		file.open(QFile::ReadOnly | QFile::Text);
		QXmlInputSource xmlInputSource(&file);
		reader.parse(xmlInputSource);

		int regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();
		int volumeWidthInRegions = volumeChangeTracker->getWrappedVolume()->getWidth() / regionSideLength;
		int volumeHeightInRegions = volumeChangeTracker->getWrappedVolume()->getHeight() / regionSideLength;
		int volumeDepthInRegions = volumeChangeTracker->getWrappedVolume()->getDepth() / regionSideLength;

		m_volRegionTimeStamps = new Volume<uint32_t>(volumeWidthInRegions, volumeHeightInRegions, volumeDepthInRegions, 0);
		m_volMapRegions = new Volume<MapRegion*>(volumeWidthInRegions, volumeHeightInRegions, volumeDepthInRegions, 0);

		volumeChangeTracker->setAllRegionsModified();

		m_pMTSE = new MultiThreadedSurfaceExtractor(volumeChangeTracker->getWrappedVolume(), qApp->settings()->value("Engine/NoOfSurfaceExtractionThreads", 2).toInt());

		return true;
	}

	void Map::updatePolyVoxGeometry()
	{
		if(!volumeResource.isNull())
		{
			timer->reset();

			int regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();
			int volumeWidthInRegions = volumeChangeTracker->getWrappedVolume()->getWidth() / regionSideLength;
			int volumeHeightInRegions = volumeChangeTracker->getWrappedVolume()->getHeight() / regionSideLength;
			int volumeDepthInRegions = volumeChangeTracker->getWrappedVolume()->getDepth() / regionSideLength;

			int counter = 0;

			//Iterate over each region in the VolumeChangeTracker
			for(PolyVox::uint16_t regionZ = 0; regionZ < volumeDepthInRegions; ++regionZ)
			{		
				for(PolyVox::uint16_t regionY = 0; regionY < volumeHeightInRegions; ++regionY)
				{
					for(PolyVox::uint16_t regionX = 0; regionX < volumeWidthInRegions; ++regionX)
					{
						//If the region has changed then we may need to add or remove MapRegion to/from the scene graph
						//if(volumeChangeTracker->getLastModifiedTimeForRegion(regionX, regionY, regionZ) > m_iRegionTimeStamps[regionX][regionY][regionZ])
						if(volumeChangeTracker->getLastModifiedTimeForRegion(regionX, regionY, regionZ) > m_volRegionTimeStamps->getVoxelAt(regionX,regionY,regionZ))
						{
							//Convert to a real PolyVox::Region
							const PolyVox::uint16_t firstX = regionX * regionSideLength;
							const PolyVox::uint16_t firstY = regionY * regionSideLength;
							const PolyVox::uint16_t firstZ = regionZ * regionSideLength;
							const PolyVox::uint16_t lastX = firstX + regionSideLength;
							const PolyVox::uint16_t lastY = firstY + regionSideLength;
							const PolyVox::uint16_t lastZ = firstZ + regionSideLength;		

							Vector3DInt16 v3dLowerCorner(firstX,firstY,firstZ);
							Vector3DInt16 v3dUpperCorner(lastX,lastY,lastZ);
							Region region(v3dLowerCorner, v3dUpperCorner);
							region.cropTo(volumeChangeTracker->getWrappedVolume()->getEnclosingRegion());

							MapRegion* pMapRegion = m_volMapRegions->getVoxelAt(regionX, regionY, regionZ);

							Vector3DInt16 v3dStart(0,0,0);
							Vector3DInt16 v3dCentre = region.getCentre();
							Vector3DInt16 v3dDiff = v3dStart - v3dCentre;

							//uint32_t uPriority = std::numeric_limits<uint32_t>::max() - (v3dDiff.getX() * v3dDiff.getX() + v3dDiff.getY() * v3dDiff.getY() + v3dDiff.getZ() * v3dDiff.getZ());

							double distanceSquared = v3dDiff.lengthSquared();
							distanceSquared = sqrt(distanceSquared);
							uint32_t uPriority = std::numeric_limits<uint32_t>::max() - static_cast<uint32_t>(distanceSquared);

							//uint32_t uPriority = firstX + firstY + firstZ;
							//uint32_t uPriority = v3dCentre.getX() + v3dCentre.getY() + v3dCentre.get();

							//uint32_t uPriority = counter;
							//counter++;

							m_pMTSE->pushTask(SurfaceExtractorTaskData(region, 0, uPriority));
							m_iNoSubmitted++;

							float fLod0ToLod1Boundary = qApp->settings()->value("Engine/Lod0ToLod1Boundary", 256.0f).toDouble();
							float fLod1ToLod2Boundary = qApp->settings()->value("Engine/Lod1ToLod2Boundary", 512.0f).toDouble();
							if(fLod0ToLod1Boundary > 0)
							{
								m_pMTSE->pushTask(SurfaceExtractorTaskData(region, 1, uPriority));
								m_iNoSubmitted++;
							}

							if(fLod1ToLod2Boundary > 0)
							{
								m_pMTSE->pushTask(SurfaceExtractorTaskData(region, 2, uPriority));
								m_iNoSubmitted++;
							}

							m_volRegionTimeStamps->setVoxelAt(regionX,regionY,regionZ,volumeChangeTracker->getLastModifiedTimeForRegion(regionX, regionY, regionZ));
						}
					}
				}
			}

			m_pMTSE->start();
		}

		/*while(true)
		{
		m_pMTSE->m_mutexCompletedTasks->lock();
		if(m_pMTSE->m_listCompletedTasks.empty())
		{
			m_pMTSE->m_mutexCompletedTasks->unlock();
			break;
		}
		SurfaceExtractorTaskData taskData = m_pMTSE->m_listCompletedTasks.front();
		m_pMTSE->m_listCompletedTasks.pop_front();
		m_pMTSE->m_mutexCompletedTasks->unlock();*/

		/*int regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();
		int volumeWidthInRegions = volumeChangeTracker->getWrappedVolume()->getWidth() / regionSideLength;
		int volumeHeightInRegions = volumeChangeTracker->getWrappedVolume()->getHeight() / regionSideLength;
		int volumeDepthInRegions = volumeChangeTracker->getWrappedVolume()->getDepth() / regionSideLength;
		int noOfRegions = volumeWidthInRegions * volumeHeightInRegions * volumeDepthInRegions * 3;*/

		while(m_pMTSE->noOfResultsAvailable() > 0)
		{
			m_iNoProcessed++;
			float fProgress = static_cast<float>(m_iNoProcessed) / static_cast<float>(m_iNoSubmitted);
			m_pThermiteGameLogic->m_loadingProgress->setExtractingSurfacePercentageDone(fProgress*100);
			if(fProgress > 0.999)
			{
				m_pThermiteGameLogic->m_loadingProgress->hide();
			}

			SurfaceExtractorTaskData result;
			result = m_pMTSE->popResult();

		int regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();
		PolyVox::uint16_t regionX = result.getRegion().getLowerCorner().getX() / regionSideLength;
		PolyVox::uint16_t regionY = result.getRegion().getLowerCorner().getY() / regionSideLength;
		PolyVox::uint16_t regionZ = result.getRegion().getLowerCorner().getZ() / regionSideLength;

		MapRegion* pMapRegion = m_volMapRegions->getVoxelAt(regionX, regionY, regionZ);
		if(pMapRegion == 0)
		{
			pMapRegion = new MapRegion(this, result.getRegion().getLowerCorner());
			m_volMapRegions->setVoxelAt(regionX, regionY, regionZ, pMapRegion);
		}

		POLYVOX_SHARED_PTR<IndexedSurfacePatch> ispWhole = result.getIndexedSurfacePatch();

		//computeNormalsForVertices(volumeChangeTracker->getWrappedVolume(), *(isp.get()), SOBEL);
		//*ispCurrent = getSmoothedSurface(*ispCurrent);
		//isp->smooth(0.3f);
		//ispCurrent->generateAveragedFaceNormals(true);

		for(std::map< std::string, std::set<PolyVox::uint8_t> >::iterator iter = m_mapMaterialIds.begin(); iter != m_mapMaterialIds.end(); iter++)
		{
			std::string materialName = iter->first;
			std::set<uint8_t> voxelValues = iter->second;

			POLYVOX_SHARED_PTR<IndexedSurfacePatch> ispSubset = ispWhole->extractSubset(voxelValues);

			switch(result.getLodLevel())
			{
			case 0:
				{

				//pMapRegion->m_renderOperationLod0[materialName] = MapRegion::buildRenderOperationFrom(*(ispSubset.get()));

				pMapRegion->update(ispWhole.get()); //This shouldn't be called in the loop - using Whole not Subset.
				break;
				}
			case 1:
				{
				//pMapRegion->m_renderOperationLod1[materialName] = MapRegion::buildRenderOperationFrom(*(ispSubset.get()));
				break;
				}
			case 2:
				{
				//pMapRegion->m_renderOperationLod2[materialName] = MapRegion::buildRenderOperationFrom(*(ispSubset.get()));
				break;
				}
			}

			pMapRegion->addSurfacePatchRenderable(materialName, *ispSubset);
		}

		//The MapRegion is now up to date. Update the time stamp to indicate this
		m_volRegionTimeStamps->setVoxelAt(regionX,regionY,regionZ,volumeChangeTracker->getLastModifiedTimeForRegion(regionX, regionY, regionZ));
		}

		/**/

		/*float timeElapsed = timer->getMilliseconds();
		if(listChangedRegionGeometry.size() > 0)
		{
		std::stringstream ss;
		ss << "Regenerated " << listChangedRegionGeometry.size() << " regions in " << timeElapsed << "ms" << std::endl;
		ss << "Averaged " << timeElapsed/listChangedRegionGeometry.size() << "ms per block";
		Ogre::LogManager::getSingleton().logMessage(ss.str()); 
		}*/
	}

	void Map::createAxis(unsigned int uWidth, unsigned int uHeight, unsigned int uDepth)
	{
		float fWidth = static_cast<float>(uWidth);
		float fHeight = static_cast<float>(uHeight);
		float fDepth = static_cast<float>(uDepth);
		float fHalfWidth = fWidth/2.0;
		float fHalfHeight = fHeight/2.0;
		float fHalfDepth = fDepth/2.0;

		float fOriginSize = 4.0f;	
		Ogre::Vector3 vecToUnitCube(0.01,0.01,0.01);

		//Create the main node for the axes
		m_axisNode = m_pOgreSceneManager->getRootSceneNode()->createChildSceneNode();

		//Create sphere representing origin
		Ogre::SceneNode* originNode = m_axisNode->createChildSceneNode();
		Ogre::Entity *originSphereEntity = m_pOgreSceneManager->createEntity( "Origin Sphere", Ogre::SceneManager::PT_CUBE );
		originSphereEntity->setMaterialName("OriginMaterial");
		originNode->attachObject(originSphereEntity);
		originNode->scale(vecToUnitCube);
		originNode->scale(fOriginSize,fOriginSize,fOriginSize);

		//Create x-axis
		Ogre::SceneNode *xAxisCylinderNode = m_axisNode->createChildSceneNode();
		Ogre::Entity *xAxisCylinderEntity = m_pOgreSceneManager->createEntity( "X Axis", Ogre::SceneManager::PT_CUBE );
		xAxisCylinderEntity->setMaterialName("XAxisMaterial");
		xAxisCylinderNode->attachObject(xAxisCylinderEntity);	
		xAxisCylinderNode->scale(vecToUnitCube);
		xAxisCylinderNode->scale(Ogre::Vector3(fWidth,1.0,1.0));
		xAxisCylinderNode->translate(Ogre::Vector3(fHalfWidth,0.0,0.0));

		//Create y-axis
		Ogre::SceneNode *yAxisCylinderNode = m_axisNode->createChildSceneNode();
		Ogre::Entity *yAxisCylinderEntity = m_pOgreSceneManager->createEntity( "Y Axis", Ogre::SceneManager::PT_CUBE );
		yAxisCylinderEntity->setMaterialName("YAxisMaterial");
		yAxisCylinderNode->attachObject(yAxisCylinderEntity);		
		yAxisCylinderNode->scale(vecToUnitCube);
		yAxisCylinderNode->scale(Ogre::Vector3(1.0,fHeight,1.0));
		yAxisCylinderNode->translate(Ogre::Vector3(0.0,fHalfHeight,0.0));

		//Create z-axis
		Ogre::SceneNode *zAxisCylinderNode = m_axisNode->createChildSceneNode();
		Ogre::Entity *zAxisCylinderEntity = m_pOgreSceneManager->createEntity( "Z Axis", Ogre::SceneManager::PT_CUBE );
		zAxisCylinderEntity->setMaterialName("ZAxisMaterial");
		zAxisCylinderNode->attachObject(zAxisCylinderEntity);
		zAxisCylinderNode->scale(vecToUnitCube);
		zAxisCylinderNode->scale(Ogre::Vector3(1.0,1.0,fDepth));
		zAxisCylinderNode->translate(Ogre::Vector3(0.0,0.0,fHalfDepth));

		//Create remainder of box		
		Ogre::ManualObject* remainingBox = m_pOgreSceneManager->createManualObject("Remaining Box");
		remainingBox->begin("BaseWhiteNoLighting",Ogre::RenderOperation::OT_LINE_LIST);
		remainingBox->position(0.0,		0.0,		0.0);		remainingBox->position(0.0,		0.0,		fDepth);
		remainingBox->position(0.0,		fHeight,	0.0);		remainingBox->position(0.0,		fHeight,	fDepth);
		remainingBox->position(fWidth,	0.0,		0.0);		remainingBox->position(fWidth,	0.0,		fDepth);
		remainingBox->position(fWidth,	fHeight,	0.0);		remainingBox->position(fWidth,	fHeight,	fDepth);

		remainingBox->position(0.0,		0.0,		0.0);		remainingBox->position(0.0,		fHeight,	0.0);
		remainingBox->position(0.0,		0.0,		fDepth);	remainingBox->position(0.0,		fHeight,	fDepth);
		remainingBox->position(fWidth,	0.0,		0.0);		remainingBox->position(fWidth,	fHeight,	0.0);
		remainingBox->position(fWidth,	0.0,		fDepth);	remainingBox->position(fWidth,	fHeight,	fDepth);

		remainingBox->position(0.0,		0.0,		0.0);		remainingBox->position(fWidth,	0.0,		0.0);
		remainingBox->position(0.0,		0.0,		fDepth);	remainingBox->position(fWidth,	0.0,		fDepth);
		remainingBox->position(0.0,		fHeight,	0.0);		remainingBox->position(fWidth,	fHeight,	0.0);
		remainingBox->position(0.0,		fHeight,	fDepth);	remainingBox->position(fWidth,	fHeight,	fDepth);
		remainingBox->end();
		Ogre::SceneNode *remainingBoxNode = m_axisNode->createChildSceneNode();
		remainingBoxNode->attachObject(remainingBox);
	}
}