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

#include "Map.h"

#include "Application.h"
#include "MapHandler.h"
#include "VolumeManager.h"

#include "VolumeChangeTracker.h"

#include "MapRegion.h"

#include "ThermiteGameLogic.h"

#ifdef ENABLE_BULLET_PHYSICS
	#include "OgreBulletDynamicsWorld.h"
#endif //ENABLE_BULLET_PHYSICS

#include <OgreSceneManagerEnumerator.h>
#include <OgreSceneManager.h>

#include <QSettings>

//using namespace Ogre;
using namespace PolyVox;

namespace Thermite
{
	Map::Map()
	{
		m_pOgreSceneManager = 0;
		//m_pOgreSceneManager = new Ogre::DefaultSceneManager("MapSceneManager");
#ifdef ENABLE_BULLET_PHYSICS
		const Ogre::Vector3 gravityVector = Ogre::Vector3 (0,-98.1,0);
		const Ogre::AxisAlignedBox bounds = Ogre::AxisAlignedBox (Ogre::Vector3 (-10000, -10000, -10000),Ogre::Vector3 (10000,  10000,  10000));
		m_pOgreBulletWorld = new OgreBulletDynamics::DynamicsWorld(m_pOgreSceneManager, bounds, gravityVector);

	/*Map::Map(Ogre::SceneManager* sceneManager, OgreBulletDynamics::DynamicsWorld *pOgreBulletWorld)
	{
		m_pOgreSceneManager = sceneManager;
		m_pOgreBulletWorld = pOgreBulletWorld;
	}*/
#else
	/*Map::Map(Ogre::SceneManager* sceneManager)
	{
		m_pOgreSceneManager = sceneManager;
	}*/
#endif //ENABLE_BULLET_PHYSICS
	}

	Map::~Map(void)
	{

	}
}