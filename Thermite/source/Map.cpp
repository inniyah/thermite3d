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

using PolyVox::uint32_t;
using PolyVox::uint16_t;
using PolyVox::uint8_t;

namespace Thermite
{
	Map::Map()
	{
		m_pOgreSceneManager = new Ogre::DefaultSceneManager("MapSceneManager");
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