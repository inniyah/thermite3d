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

#include "Perlin.h"
#include "SurfaceMeshExtractionTask.h"
#include "SurfaceMeshDecimationTask.h"
#include "TaskProcessorThread.h"

#include <QSettings>
#include <QThreadPool>

using namespace PolyVox;

namespace Thermite
{
	Node* Node::startNode = 0;
	Node* Node::endNode = 0;

	class NodeSort
	{
	public:
		bool operator() (const Node* lhs, const Node* rhs) const
		{
			return lhs->g() + lhs->h() > rhs->g() + rhs->h();
		}
	};

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

		uint32_t dimensions[3] = {mVolumeWidthInRegions, mVolumeHeightInRegions, mVolumeDepthInRegions}; // Array dimensions
		mLastModifiedArray.resize(dimensions); std::fill(mLastModifiedArray.getRawData(), mLastModifiedArray.getRawData() + mLastModifiedArray.getNoOfElements(), globals.timeStamp());
		mExtractionStartedArray.resize(dimensions); std::fill(mExtractionStartedArray.getRawData(), mExtractionStartedArray.getRawData() + mExtractionStartedArray.getNoOfElements(), 0);
		mExtractionFinishedArray.resize(dimensions); std::fill(mExtractionFinishedArray.getRawData(), mExtractionFinishedArray.getRawData() + mExtractionFinishedArray.getNoOfElements(), 0);
		m_volSurfaceMeshes.resize(dimensions); std::fill(m_volSurfaceMeshes.getRawData(), m_volSurfaceMeshes.getRawData() + m_volSurfaceMeshes.getNoOfElements(), (SurfaceMesh<PositionMaterial>*)0);
		mRegionBeingExtracted.resize(dimensions); std::fill(mRegionBeingExtracted.getRawData(), mRegionBeingExtracted.getRawData() + mRegionBeingExtracted.getNoOfElements(), 0);
		m_volSurfaceDecimators.resize(dimensions); std::fill(m_volSurfaceDecimators.getRawData(), m_volSurfaceDecimators.getRawData() + m_volSurfaceDecimators.getNoOfElements(), (Thermite::SurfaceMeshDecimationTask*)0);
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
			setModified(true);
		}
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
		//Initialise to failure
		/*std::pair<bool, QVector3D> result;
		result.first = false;
		result.second = QVector3D(0,0,0);*/

		QVector3D result = QVector3D(0,0,0);

		//Ensure the voume is valid
		PolyVox::Volume<Material8>* pVolume = m_pPolyVoxVolume;
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
		int startX = start.x() + 0.5f;
		int startZ = start.z() + 0.5f;
		int endX = end.x() + 0.5f;
		int endZ = end.z() + 0.5f;

		QVariantList result;

		Node* nodes[256][256];
		//std::fill(nodes[0][0], nodes[255][255], 0);
		for(int z = 0; z < 256; z++)
		{
			for(int x = 0; x < 256; x++)
			{
				Node* node = new Node;

				node->position.setX(x);
				node->position.setY(start.y() + 0.5f);
				node->position.setZ(z);
				node->gVal = 1000000;
				node->parent = 0;

				nodes[x][z] = node;
			}
		}

		Node* startNode = nodes[startX][startZ];
		Node* endNode = nodes[endX][endZ];

		Node::startNode = startNode;
		Node::endNode = endNode;

		std::vector<Node*> open;
		//make_heap(open.begin(), open.end());
		std::set<Node*> closed;

		startNode->gVal = 0;

		open.push_back(startNode);
		push_heap(open.begin(), open.end(), NodeSort());

		while(open[0] != endNode)
		{
			Node* current = open[0];
			pop_heap(open.begin(), open.end(), NodeSort());
			open.pop_back();

			int currentX = current->position.getX();
			int currentZ = current->position.getZ();

			//closed.push_back(current);
			closed.insert(current);

			//Plus X
			//Node* plusX = nodes[currentX + 1][currentZ];
			processNeighbour(current, nodes[currentX + 1][currentZ], open, closed);
			processNeighbour(current, nodes[currentX - 1][currentZ], open, closed);
			processNeighbour(current, nodes[currentX][currentZ + 1], open, closed);
			processNeighbour(current, nodes[currentX][currentZ - 1], open, closed);
			
		}

		Node* n = endNode;
		while(n != 0)
		{
			result.prepend(QVector3D(n->position.getX(), n->position.getY(), n->position.getZ()));
			n = n->parent;
		}

		//result.append(QVector3D(128,end.y(),128));
		//result.append(end);

		emit foundPath(result);

		//return result;
	}

	void Volume::processNeighbour(Node* current, Node* neighbour, std::vector<Node*>& open, std::set<Node*>& closed)
	{
		Material8 voxel = m_pPolyVoxVolume->getVoxelAt(neighbour->position.getX(), neighbour->position.getY(), neighbour->position.getZ());
		int cost = current->gVal + 1.0f;

		if(voxel.getMaterial() > 0)
		{
			cost = 1000000;
		}

		std::vector<Node*>::iterator openIter = std::find(open.begin(), open.end(), neighbour);
		if(openIter != open.end())
		{
			if(cost < neighbour->gVal)
			{
				open.erase(openIter);
				make_heap(open.begin(), open.end(), NodeSort());
				openIter = open.end();
			}
		}

		std::set<Node*>::iterator closedIter = std::find(closed.begin(), closed.end(), neighbour);
		if(closedIter != closed.end())
		{
			if(cost < neighbour->gVal)
			{
				//Probably shouldn't happen?
				closed.erase(closedIter);
				closedIter = closed.end();
			}
		}

		if((openIter == open.end()) && (closedIter == closed.end()))
		{
			neighbour->gVal = cost;

			open.push_back(neighbour);
			push_heap(open.begin(), open.end(), NodeSort());

			neighbour->parent = current;
		}
	}
}
