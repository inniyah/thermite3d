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

#ifndef __ForwardDeclarations_H__
#define __ForwardDeclarations_H__

#include "PolyVoxImpl/CPlusPlusZeroXSupport.h"
const PolyVox::uint16_t THERMITE_VOLUME_SIDE_LENGTH_POWER = 8;
const PolyVox::uint16_t THERMITE_VOLUME_SIDE_LENGTH = (0x0001 << THERMITE_VOLUME_SIDE_LENGTH_POWER);

const PolyVox::uint16_t THERMITE_REGION_SIDE_LENGTH_POWER = 5;
const PolyVox::uint16_t THERMITE_REGION_SIDE_LENGTH = (0x0001 << THERMITE_REGION_SIDE_LENGTH_POWER);
const PolyVox::uint16_t THERMITE_VOLUME_SIDE_LENGTH_IN_REGIONS = (THERMITE_VOLUME_SIDE_LENGTH >> THERMITE_REGION_SIDE_LENGTH_POWER);



namespace Thermite
{
	class ApplicationGameLogic;
	class CannonController;
	class GameState;
	//class IndexedSurfacePatchCollisionShape;
	class LoadSceneMenuPage;
	class MainMenuPage;
	class Map;
	class MenuPage;
	class MenuState;
	class PhysicalObject;
	class PlayState;
	class RunnerThread;
	class State;
	class StateManager;
	class SurfacePatchRenderable;
	//class VolumeManager;
	//class VolumeResourse;
	//class VolumeSerializer;
	class MapRegion;

	class LoadMapWidget;
	class MoviePlayer;
	class Shell;
	class ThermiteGameLogic;
}

#endif
