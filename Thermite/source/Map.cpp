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

#include "TimeStampedSurfacePatchCache.h"
#include "TimeStampedRenderOperationCache.h"

#include "VolumeChangeTracker.h"
#include "SurfaceExtractors.h"

#include "SurfacePatchRenderable.h"
#include "WorldRegion.h"

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
{
	memset(m_iRegionTimeStamps, 0xFF, sizeof(m_iRegionTimeStamps));

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

	return true;
}

void Map::updatePolyVoxGeometry()
{
	if(!volumeResource.isNull())
	{
		timer->reset();

		//Iterate over each region in the VolumeChangeTracker
		for(PolyVox::uint16_t regionZ = 0; regionZ < THERMITE_VOLUME_SIDE_LENGTH_IN_REGIONS; ++regionZ)
		{		
			for(PolyVox::uint16_t regionY = 0; regionY < THERMITE_VOLUME_SIDE_LENGTH_IN_REGIONS; ++regionY)
			{
				for(PolyVox::uint16_t regionX = 0; regionX < THERMITE_VOLUME_SIDE_LENGTH_IN_REGIONS; ++regionX)
				{
					//If the region has changed then we may need to add or remove WorldRegion to/from the scene graph
					if(volumeChangeTracker->getLastModifiedTimeForRegion(regionX, regionY, regionZ) > m_iRegionTimeStamps[regionX][regionY][regionZ])
					{
						//Convert to a real PolyVox::Region
						const PolyVox::uint16_t firstX = regionX * THERMITE_REGION_SIDE_LENGTH;
						const PolyVox::uint16_t firstY = regionY * THERMITE_REGION_SIDE_LENGTH;
						const PolyVox::uint16_t firstZ = regionZ * THERMITE_REGION_SIDE_LENGTH;
						const PolyVox::uint16_t lastX = firstX + THERMITE_REGION_SIDE_LENGTH;
						const PolyVox::uint16_t lastY = firstY + THERMITE_REGION_SIDE_LENGTH;
						const PolyVox::uint16_t lastZ = firstZ + THERMITE_REGION_SIDE_LENGTH;

						Vector3DInt32 v3dLowerCorner(firstX,firstY,firstZ);
						Vector3DInt32 v3dUpperCorner(lastX,lastY,lastZ);
						Region region(v3dLowerCorner, v3dUpperCorner);
						region.cropTo(volumeChangeTracker->getWrappedVolume()->getEnclosingRegion());						

						//Enlarging the region is required because low res meshes can cover more volume.
						Region regToCheck = region;
						regToCheck.shiftUpperCorner(Vector3DInt32(7,7,7));
						regToCheck.cropTo(volumeChangeTracker->getWrappedVolume()->getEnclosingRegion());
						
						//There are two situations we are concerned with. If a region is homogenous then
						//we make sure it is in the scene graph (it might already be). If a region is not
						//homogenous we make sure it is not in the scene graph (it might already not be).
						if(volumeChangeTracker->getWrappedVolume()->isRegionHomogenous(regToCheck))
						{
							//World region should be removed if it exists.							
							std::map<PolyVox::Vector3DInt32, WorldRegion*>::iterator worldRegionIter = m_mapWorldRegions.find(v3dLowerCorner);
							if(worldRegionIter != m_mapWorldRegions.end())
							{
								//Delete the world region and remove the pointer from the map.
								delete worldRegionIter->second;
								m_mapWorldRegions.erase(worldRegionIter);
							}
						}		
						else
						{
							//World region should be added if it doesn't exist.
							WorldRegion* pWorldRegion;
							std::map<PolyVox::Vector3DInt32, WorldRegion*>::iterator worldRegionIter = m_mapWorldRegions.find(v3dLowerCorner);
							if(worldRegionIter == m_mapWorldRegions.end())
							{
								pWorldRegion = new WorldRegion(this, v3dLowerCorner);				
								//pWorldRegion->setPosition(v3dLowerCorner);
								m_mapWorldRegions.insert(std::make_pair(v3dLowerCorner, pWorldRegion));
							}
							else
							{
								pWorldRegion = worldRegionIter->second;
							}

							//Regardless of whether it has just been created, we need to make sure the physics geometry is up to date.
							//For the graphics geometry this is done automatically each time Ogre tries to render a SurfacePatchRenderable.
							if(qApp->settings()->value("Physics/SimulatePhysics", false).toBool())
							{
								IndexedSurfacePatch* isp = TimeStampedSurfacePatchCache::getInstance()->getIndexedSurfacePatch(v3dLowerCorner, 1);
								pWorldRegion->setPhysicsData(Ogre::Vector3(v3dLowerCorner.getX(),v3dLowerCorner.getY(),v3dLowerCorner.getZ()), *isp);
							}
						}

						//The WorldRegion is now up to date. Update the time stamp to indicate this
						m_iRegionTimeStamps[regionX][regionY][regionZ] = volumeChangeTracker->getLastModifiedTimeForRegion(regionX, regionY, regionZ);
					}
				}
			}
		}
	}

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