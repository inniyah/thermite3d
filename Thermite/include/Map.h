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

#ifndef __THERMITE_MAP_H__
#define __THERMITE_MAP_H__

#include "ThermiteForwardDeclarations.h"
#include "VolumeResource.h"
#include "VolumeChangeTracker.h"

#include "SurfaceExtractorTaskData.h"
#include "TaskProcessorThread.h"

#include "PolyVoxForwardDeclarations.h"

#ifdef ENABLE_BULLET_PHYSICS
	#include "OgreBulletDynamicsWorld.h"
	#include "OgreBulletDynamicsRigidBody.h"
#endif //ENABLE_BULLET_PHYSICS

#include <OgrePrerequisites.h>

#include <map>

#include "Object.h"

#include <QScriptEngine>
#include <QVector3D>

namespace Thermite
{
	class Map : public Object
	{
		Q_OBJECT

	public:
		Map(QObject* parent = 0);
		~Map(void);

		void initialise(void);

		void updatePolyVoxGeometry(Ogre::Vector3 cameraPos);

	public slots:
		void createSphereAt(QVector3D centre, float radius, int value, bool bPaintMode);
		QVector3D getRayVolumeIntersection(QVector3D rayOrigin, const QVector3D& rayDir);

		void uploadSurfaceExtractorResult(SurfaceExtractorTaskData result);
		void uploadSurfaceDecimatorResult(SurfaceExtractorTaskData result);
		void uploadSurfaceMesh(const PolyVox::SurfaceMesh& mesh, PolyVox::Region region);

	public:
		Ogre::SceneManager* m_pOgreSceneManager;

		TaskProcessorThread* m_backgroundThread;

		VolumeResourcePtr volumeResource;

		std::map< std::string, std::set<uint8_t> > m_mapMaterialIds;	

		PolyVox::VolumeChangeTracker<PolyVox::MaterialDensityPair44>* volumeChangeTracker;

		PolyVox::Volume<MapRegion*>* m_volMapRegions;	
		PolyVox::Volume<uint32_t>* m_volRegionTimeStamps;
		PolyVox::Volume<bool>* m_volRegionBeingProcessed;
		PolyVox::Volume<SurfaceMeshDecimationTask*>* m_volSurfaceDecimators;
	};	
}

Q_SCRIPT_DECLARE_QMETAOBJECT(Thermite::Map, QObject*)


#endif