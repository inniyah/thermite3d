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

#include "World.h"

#include "Application.h"
#include "DotSceneWithVolumeHandler.h"
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

using namespace Ogre;
using namespace PolyVox;
using namespace OgreBulletDynamics;
using namespace OgreBulletCollisions;

World::World(Ogre::Vector3 vecGravity, Ogre::AxisAlignedBox boxPhysicsBounds, Ogre::Real rVoxelSize, Ogre::SceneManager* sceneManager)
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

World::~World(void)
{

}

void World::initialisePhysics(void)
{
	const Ogre::Vector3 gravityVector = Ogre::Vector3 (0,-98.1,0);
	const Ogre::AxisAlignedBox bounds = Ogre::AxisAlignedBox (Ogre::Vector3 (-10000, -10000, -10000),Ogre::Vector3 (10000,  10000,  10000));
	m_pOgreBulletWorld = new DynamicsWorld(m_pOgreSceneManager, bounds, gravityVector);
}

bool World::loadScene(const Ogre::String& filename)
{
	/*DotSceneWithVolumeHandler handler(m_pOgreSceneManager);
	QXmlSimpleReader reader;
	reader.setContentHandler(&handler);
	reader.setErrorHandler(&handler);

	QFile file("C:\\ogreaddons\\dotsceneformat\\Source\\Common\\trunk\\main\\XmlNodeProcessor\\test.scene");
    file.open(QFile::ReadOnly | QFile::Text);
	QXmlInputSource xmlInputSource(&file);
    reader.parse(xmlInputSource);

	return 0;*/


	volumeResource = VolumeManager::getSingletonPtr()->load(filename + ".volume", "General");
	if(volumeResource.isNull())
	{
		LogManager::getSingleton().logMessage("Failed to load volume");
	}

	//volumeResource->volume->tidy();
	volumeChangeTracker->setVolumeData(volumeResource->volume);
	//volumeChangeTracker->volumeData = volumeResource->volume;

	//volumeChangeTracker->volumeData->tidy();

	volumeChangeTracker->setAllRegionsModified();

	TimeStampedSurfacePatchCache::getInstance()->m_vctTracker = volumeChangeTracker;
	TimeStampedRenderOperationCache::getInstance()->m_vctTracker = volumeChangeTracker;

	return true;
}

void World::updatePolyVoxGeometry()
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
							pWorldRegion->setPhysicsData(Vector3(v3dLowerCorner.getX(),v3dLowerCorner.getY(),v3dLowerCorner.getZ()), *isp);
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
