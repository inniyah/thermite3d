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

#include "ThermiteGameLogic.h"

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
#ifdef ENABLE_BULLET_PHYSICS
	Map::Map(Ogre::SceneManager* sceneManager, OgreBulletDynamics::DynamicsWorld *pOgreBulletWorld)
	{
		m_pOgreSceneManager = sceneManager;
		m_pOgreBulletWorld = pOgreBulletWorld;
	}
#else
	Map::Map(Ogre::SceneManager* sceneManager)
	{
		m_pOgreSceneManager = sceneManager;
	}
#endif //ENABLE_BULLET_PHYSICS

	Map::~Map(void)
	{

	}

	bool Map::loadScene(const Ogre::String& filename)
	{
		

		return true;
	}
}