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

#ifndef __ForwardDeclarations_H__
#define __ForwardDeclarations_H__

#include "PolyVoxImpl/CPlusPlusZeroXSupport.h"
const PolyVox::uint16_t THERMITE_VOLUME_SIDE_LENGTH_POWER = 8;
const PolyVox::uint16_t THERMITE_VOLUME_SIDE_LENGTH = (0x0001 << THERMITE_VOLUME_SIDE_LENGTH_POWER);

const PolyVox::uint16_t THERMITE_REGION_SIDE_LENGTH_POWER = 5;
const PolyVox::uint16_t THERMITE_REGION_SIDE_LENGTH = (0x0001 << THERMITE_REGION_SIDE_LENGTH_POWER);
const PolyVox::uint16_t THERMITE_VOLUME_SIDE_LENGTH_IN_REGIONS = (THERMITE_VOLUME_SIDE_LENGTH >> THERMITE_REGION_SIDE_LENGTH_POWER);

//class Application;
class GameState;
//class IndexedSurfacePatchCollisionShape;
class LoadSceneMenuPage;
class MainMenuPage;
class Map;
class MenuPage;
class MenuState;
class PhysicalObject;
class PlayState;
class State;
class StateManager;
class SurfacePatchRenderable;
//class VolumeManager;
//class VolumeResourse;
//class VolumeSerializer;
class MapRegion;

namespace Thermite
{
	class LoadMapWidget;
	class MoviePlayer;
	class Shell;
	class ThermiteGameLogic;
}

#endif
