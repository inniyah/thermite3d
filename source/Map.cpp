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

#include "PolyVoxUtil/VolumeChangeTracker.h"
#include "PolyVoxCore/SurfaceExtractors.h"

#include "SurfacePatchRenderable.h"
#include "WorldRegion.h"

#include "OgreBulletDynamicsWorld.h"

#include "Shapes/OgreBulletCollisionsBoxShape.h"


#include <OgreSceneManagerEnumerator.h>
#include <OgreSceneManager.h>
#include <OgreTimer.h>

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

	volumeChangeTracker = new VolumeChangeTracker();

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
		for(PolyVox::uint16 regionZ = 0; regionZ < POLYVOX_VOLUME_SIDE_LENGTH_IN_REGIONS; ++regionZ)
		{		
			for(PolyVox::uint16 regionY = 0; regionY < POLYVOX_VOLUME_SIDE_LENGTH_IN_REGIONS; ++regionY)
			{
				for(PolyVox::uint16 regionX = 0; regionX < POLYVOX_VOLUME_SIDE_LENGTH_IN_REGIONS; ++regionX)
				{
					//If the region has changed then we may need to add or remove WorldRegion to/from the scene graph
					if(volumeChangeTracker->getLastModifiedTimeForRegion(regionX, regionY, regionZ) > m_iRegionTimeStamps[regionX][regionY][regionZ])
					{
						//Convert to a real PolyVox::Region
						const PolyVox::uint16 firstX = regionX * POLYVOX_REGION_SIDE_LENGTH;
						const PolyVox::uint16 firstY = regionY * POLYVOX_REGION_SIDE_LENGTH;
						const PolyVox::uint16 firstZ = regionZ * POLYVOX_REGION_SIDE_LENGTH;
						const PolyVox::uint16 lastX = firstX + POLYVOX_REGION_SIDE_LENGTH;
						const PolyVox::uint16 lastY = firstY + POLYVOX_REGION_SIDE_LENGTH;
						const PolyVox::uint16 lastZ = firstZ + POLYVOX_REGION_SIDE_LENGTH;

						Vector3DInt32 v3dLowerCorner(firstX,firstY,firstZ);
						Vector3DInt32 v3dUpperCorner(lastX,lastY,lastZ);
						Region region(v3dLowerCorner, v3dUpperCorner);
						region.cropTo(volumeChangeTracker->getVolumeData()->getEnclosingRegion());						

						//Enlarging the region is required because low res meshes can cover more volume.
						Region regToCheck = region;
						regToCheck.shiftUpperCorner(Vector3DInt32(7,7,7));
						regToCheck.cropTo(volumeChangeTracker->getVolumeData()->getEnclosingRegion());
						
						//There are two situations we are concerned with. If a region has is homogenous then
						//we make sure it is in the scene graph (it might already be). If a region is not
						//homogenous we make sure it is not in the scene graph (it might already not be).
						if(volumeChangeTracker->getVolumeData()->isRegionHomogenous(regToCheck))
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
							IndexedSurfacePatch* isp = TimeStampedSurfacePatchCache::getInstance()->getIndexedSurfacePatch(v3dLowerCorner, 1);
							pWorldRegion->setPhysicsData(Ogre::Vector3(v3dLowerCorner.getX(),v3dLowerCorner.getY(),v3dLowerCorner.getZ()), *isp);
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

void Map::createAxis(unsigned int uSideLength)
{
	float fSideLength = static_cast<float>(uSideLength);
	float fHalfSideLength = fSideLength/2.0;
	Ogre::Vector3 vecOriginAndConeScale(4.0,4.0,4.0);

	//Create the main node for the axes
	m_axisNode = m_pOgreSceneManager->getRootSceneNode()->createChildSceneNode();

	//Create sphere representing origin
	Ogre::SceneNode* originNode = m_axisNode->createChildSceneNode();
	Ogre::Entity *originSphereEntity = m_pOgreSceneManager->createEntity( "Origin Sphere", "Sphere.mesh" );
	originSphereEntity->setMaterialName("WhiteMaterial");
	originSphereEntity->setCastShadows(false);
	originNode->attachObject(originSphereEntity);
	originNode->scale(vecOriginAndConeScale);

	//Create arrow body for x-axis
	Ogre::SceneNode *xAxisCylinderNode = m_axisNode->createChildSceneNode();
	Ogre::Entity *xAxisCylinderEntity = m_pOgreSceneManager->createEntity( "X Axis Cylinder", "Cylinder.mesh" );
	xAxisCylinderEntity->setMaterialName("RedMaterial");
	xAxisCylinderEntity->setCastShadows(false);
	xAxisCylinderNode->attachObject(xAxisCylinderEntity);			
	xAxisCylinderNode->scale(Ogre::Vector3(1.0,1.0,fHalfSideLength-4.0));
	xAxisCylinderNode->translate(Ogre::Vector3(fHalfSideLength,0.0,0.0));
	xAxisCylinderNode->rotate(Ogre::Vector3::UNIT_Y, Ogre::Radian(1.5707));

	//Create arrow head for x-axis
	Ogre::SceneNode *xAxisConeNode = m_axisNode->createChildSceneNode();
	Ogre::Entity *xAxisConeEntity = m_pOgreSceneManager->createEntity( "X Axis Cone", "Cone.mesh" );
	xAxisConeEntity->setMaterialName("RedMaterial");
	xAxisConeEntity->setCastShadows(false);
	xAxisConeNode->attachObject(xAxisConeEntity);		
	xAxisConeNode->rotate(Ogre::Vector3::UNIT_Y, Ogre::Radian(1.5707));
	xAxisConeNode->scale(vecOriginAndConeScale);
	xAxisConeNode->translate(Ogre::Vector3(fSideLength-4.0,0.0,0.0));

	//Create arrow body for y-axis
	Ogre::SceneNode *yAxisCylinderNode = m_axisNode->createChildSceneNode();
	Ogre::Entity *yAxisCylinderEntity = m_pOgreSceneManager->createEntity( "Y Axis Cylinder", "Cylinder.mesh" );
	yAxisCylinderEntity->setMaterialName("GreenMaterial");
	yAxisCylinderEntity->setCastShadows(false);
	yAxisCylinderNode->attachObject(yAxisCylinderEntity);		
	yAxisCylinderNode->scale(Ogre::Vector3(1.0,1.0,fHalfSideLength-4.0));
	yAxisCylinderNode->translate(Ogre::Vector3(0.0,fHalfSideLength,0.0));
	yAxisCylinderNode->rotate(Ogre::Vector3::UNIT_X, Ogre::Radian(1.5707));

	//Create arrow head for y-axis
	Ogre::SceneNode *yAxisConeNode = m_axisNode->createChildSceneNode();
	Ogre::Entity *yAxisConeEntity = m_pOgreSceneManager->createEntity( "Y Axis Cone", "Cone.mesh" );
	yAxisConeEntity->setMaterialName("GreenMaterial");
	yAxisConeEntity->setCastShadows(false);
	yAxisConeNode->attachObject(yAxisConeEntity);		
	yAxisConeNode->rotate(Ogre::Vector3::UNIT_X, Ogre::Radian(-1.5707));
	yAxisConeNode->scale(vecOriginAndConeScale);
	yAxisConeNode->translate(Ogre::Vector3(0.0,fSideLength-4.0,0.0));

	//Create arrow body for z-axis
	Ogre::SceneNode *zAxisCylinderNode = m_axisNode->createChildSceneNode();
	Ogre::Entity *zAxisCylinderEntity = m_pOgreSceneManager->createEntity( "Z Axis Cylinder", "Cylinder.mesh" );
	zAxisCylinderEntity->setMaterialName("BlueMaterial");
	zAxisCylinderEntity->setCastShadows(false);
	zAxisCylinderNode->attachObject(zAxisCylinderEntity);
	zAxisCylinderNode->translate(Ogre::Vector3(0.0,0.0,fHalfSideLength));
	zAxisCylinderNode->scale(Ogre::Vector3(1.0,1.0,fHalfSideLength-4.0));	

	//Create arrow head for z-axis
	Ogre::SceneNode *zAxisConeNode = m_axisNode->createChildSceneNode();
	Ogre::Entity *zAxisConeEntity = m_pOgreSceneManager->createEntity( "Z Axis Cone", "Cone.mesh" );
	zAxisConeEntity->setMaterialName("BlueMaterial");
	zAxisConeEntity->setCastShadows(false);
	zAxisConeNode->attachObject(zAxisConeEntity);
	zAxisConeNode->translate(Ogre::Vector3(0.0,0.0,fSideLength-4.0));
	zAxisConeNode->scale(vecOriginAndConeScale);

	//Create remainder of box		
	Ogre::ManualObject* remainingBox = m_pOgreSceneManager->createManualObject("Remaining Box");
	remainingBox->begin("BaseWhiteNoLighting",Ogre::RenderOperation::OT_LINE_LIST);
	remainingBox->position(0.0,			0.0,			0.0			);	remainingBox->position(0.0,			0.0,			fSideLength	);
	remainingBox->position(0.0,			fSideLength,	0.0			);	remainingBox->position(0.0,			fSideLength,	fSideLength	);
	remainingBox->position(fSideLength, 0.0,			0.0			);	remainingBox->position(fSideLength, 0.0,			fSideLength	);
	remainingBox->position(fSideLength, fSideLength,	0.0			);	remainingBox->position(fSideLength, fSideLength,	fSideLength	);

	remainingBox->position(0.0,			0.0,			0.0			);	remainingBox->position(0.0,			fSideLength,	0.0			);
	remainingBox->position(0.0,			0.0,			fSideLength	);	remainingBox->position(0.0,			fSideLength,	fSideLength	);
	remainingBox->position(fSideLength, 0.0,			0.0			);	remainingBox->position(fSideLength, fSideLength,	0.0			);
	remainingBox->position(fSideLength, 0.0,			fSideLength	);	remainingBox->position(fSideLength, fSideLength,	fSideLength	);

	remainingBox->position(0.0,			0.0,			0.0			);	remainingBox->position(fSideLength, 0.0,			0.0			);
	remainingBox->position(0.0,			0.0,			fSideLength	);	remainingBox->position(fSideLength, 0.0,			fSideLength	);
	remainingBox->position(0.0,			fSideLength,	0.0			);	remainingBox->position(fSideLength, fSideLength,	0.0			);
	remainingBox->position(0.0,			fSideLength,	fSideLength	);	remainingBox->position(fSideLength, fSideLength,	fSideLength	);
	remainingBox->end();
	remainingBox->setCastShadows(false);
	Ogre::SceneNode *remainingBoxNode = m_axisNode->createChildSceneNode();
	remainingBoxNode->attachObject(remainingBox);
}