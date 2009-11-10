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

#ifndef THERMITEGAMELOGIC_H_
#define THERMITEGAMELOGIC_H_

#include "AnyOption.h"
#include "GameLogic.h"
#include "LoadingProgress.h"
#include "MultiThreadedSurfaceExtractor.h"
#include "Serialization.h"
#include "VolumeChangeTracker.h"

#include "Map.h"
#include "PhysicalEntity.h"
#include "ThermiteForwardDeclarations.h"

#include "QtOgreForwardDeclarations.h"

#include <OgrePrerequisites.h>

#include <QHash>

#include <QTime>

#include <QMovie>
#include <QLabel>

#include <list>
#include <queue>

namespace Thermite
{
	enum KeyStates
	{
		KS_RELEASED,
		KS_PRESSED
	};

	class ThermiteGameLogic : public QtOgre::GameLogic
	{
	public:
		ThermiteGameLogic(void);

		void initialise(void);
		void update(void);
		void shutdown(void);

		QtOgre::Log* thermiteLog(void);

		void loadMap(QString strMapName);

		void setVolumeLoadProgress(float fProgress);

		void reloadShaders(void);

	public:
		void addResourceDirectory(const QString& directoryName);

		void createAxis(unsigned int uWidth, unsigned int uHeight, unsigned int uDepth);

		
		QTime* mTime;

		int mLastFrameTime;
		int mCurrentTime;
		float mTimeElapsedInSeconds;

		unsigned int mCurrentFrameNumber;

		

		MoviePlayer* mMoviePlayer;

		Ogre::Camera* mDummyCamera;
		Ogre::Viewport* mMainViewport;
		Ogre::Camera* mActiveCamera;
		Ogre::SceneManager* m_pDummyOgreSceneManager;
		Ogre::SceneManager* m_pActiveOgreSceneManager;
		QtOgre::Log* mThermiteLog;

		//Thermite stuff
		Map* mMap;

		std::queue<PhysicalObject*> m_queueObjects;

		//Ogre::Entity* mCannon;
		

		LoadingProgress* m_loadingProgress;

		QMovie* m_pThermiteLogoMovie;
		QLabel* m_pThermiteLogoLabel;

		Ogre::SceneNode* m_axisNode;

		bool bLoadComplete;

		AnyOption m_commandLineArgs;

	public:

		void initialisePhysics(void);

		void updatePolyVoxGeometry();

		std::pair<bool, Ogre::Vector3> getRayVolumeIntersection(const Ogre::Ray& ray);

		PolyVox::VolumeChangeTracker* volumeChangeTracker;

#ifdef ENABLE_BULLET_PHYSICS
		OgreBulletDynamics::DynamicsWorld *m_pOgreBulletWorld;
#endif //ENABLE_BULLET_PHYSICS

		PolyVox::Volume<MapRegion*>* m_volMapRegions;

		MultiThreadedSurfaceExtractor* m_pMTSE;		

		PolyVox::Volume<PolyVox::uint32_t>* m_volRegionTimeStamps;

		int m_iNoProcessed;
		int m_iNoSubmitted;
	};
}

#endif /*DEMOGAMELOGIC_H_*/