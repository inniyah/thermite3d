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


#include <OgreSceneManagerEnumerator.h>
#include <OgreSceneManager.h>
#include <OgreTimer.h>

#include <QSettings>

//using namespace Ogre;
using namespace PolyVox;
using namespace OgreBulletDynamics;
using namespace OgreBulletCollisions;

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
	MapHandler handler(this);
	QXmlSimpleReader reader;
	reader.setContentHandler(&handler);
	reader.setErrorHandler(&handler);

	QFile file("..\\share\\thermite\\Ogre\\maps\\load_me.map");
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

	m_pMTSE = new MultiThreadedSurfaceExtractor(volumeChangeTracker->getWrappedVolume(), 2);

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

						m_pMTSE->addTask(region, 0);
						m_pMTSE->addTask(region, 1);
						m_pMTSE->addTask(region, 2);

						m_volRegionTimeStamps->setVoxelAt(regionX,regionY,regionZ,volumeChangeTracker->getLastModifiedTimeForRegion(regionX, regionY, regionZ));
					}
				}
			}
		}
	}

	while(true)
	{
	m_pMTSE->m_mutexCompletedTasks->lock();
	if(m_pMTSE->m_listCompletedTasks.empty())
	{
		m_pMTSE->m_mutexCompletedTasks->unlock();
		break;
	}
	TaskData taskData = m_pMTSE->m_listCompletedTasks.front();
	m_pMTSE->m_listCompletedTasks.pop_front();
	m_pMTSE->m_mutexCompletedTasks->unlock();

	int regionSideLength = qApp->settings()->value("Engine/RegionSideLength", 64).toInt();
	PolyVox::uint16_t regionX = taskData.m_regToProcess.getLowerCorner().getX() / regionSideLength;
	PolyVox::uint16_t regionY = taskData.m_regToProcess.getLowerCorner().getY() / regionSideLength;
	PolyVox::uint16_t regionZ = taskData.m_regToProcess.getLowerCorner().getZ() / regionSideLength;

	MapRegion* pMapRegion = m_volMapRegions->getVoxelAt(regionX, regionY, regionZ);
	if(pMapRegion == 0)
	{
		pMapRegion = new MapRegion(this, taskData.m_regToProcess.getLowerCorner());
		m_volMapRegions->setVoxelAt(regionX, regionY, regionZ, pMapRegion);
	}

	POLYVOX_SHARED_PTR<IndexedSurfacePatch> isp;

	isp = taskData.m_ispResult;

	switch(taskData.m_uLodLevel)
	{
	case 0:
		pMapRegion->m_renderOperationLod0 = MapRegion::buildRenderOperationFrom(*(isp.get()));
		pMapRegion->update(isp.get());
		break;
	case 1:
		pMapRegion->m_renderOperationLod1 = MapRegion::buildRenderOperationFrom(*(isp.get()));
		break;
	case 2:
		pMapRegion->m_renderOperationLod2 = MapRegion::buildRenderOperationFrom(*(isp.get()));
		break;
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