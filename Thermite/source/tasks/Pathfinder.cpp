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

#include "Pathfinder.h"

#include "Material.h"

using namespace PolyVox;

namespace PolyVox
{
	const Vector3DInt16 arrayPathfinderFaces[6] =
	{
		Vector3DInt16(0, 0, -1),
		Vector3DInt16(0, 0, +1),
		Vector3DInt16(0, -1, 0),
		Vector3DInt16(0, +1, 0),
		Vector3DInt16(-1, 0, 0),
		Vector3DInt16(+1, 0, 0)
	};

	const Vector3DInt16 arrayPathfinderEdges[12] =
	{
		Vector3DInt16(0, -1, -1),
		Vector3DInt16(0, -1, +1),
		Vector3DInt16(0, +1, -1),
		Vector3DInt16(0, +1, +1),
		Vector3DInt16(-1, 0, -1),
		Vector3DInt16(-1, 0, +1),
		Vector3DInt16(+1, 0, -1),
		Vector3DInt16(+1, 0, +1),
		Vector3DInt16(-1, -1, 0),
		Vector3DInt16(-1, +1, 0),
		Vector3DInt16(+1, -1, 0),
		Vector3DInt16(+1, +1, 0)
	};

	const Vector3DInt16 arrayPathfinderCorners[8] =
	{
		Vector3DInt16(-1, -1, -1),
		Vector3DInt16(-1, -1, +1),
		Vector3DInt16(-1, +1, -1),
		Vector3DInt16(-1, +1, +1),
		Vector3DInt16(+1, -1, -1),
		Vector3DInt16(+1, -1, +1),
		Vector3DInt16(+1, +1, -1),
		Vector3DInt16(+1, +1, +1)
	};

	float SixConnectedCost(const Vector3DInt16& a, const Vector3DInt16& b)
	{
		//This is the only heuristic I'm sure of - just use the manhatten distance for the 6-connected case.
		uint16_t faceSteps = abs(a.getX()-b.getX()) + abs(a.getY()-b.getY()) + abs(a.getZ()-b.getZ());

		return faceSteps * 1.0f;
	}

	float EighteenConnectedCost(const Vector3DInt16& a, const Vector3DInt16& b)
	{
		//I'm not sure of the correct heuristic for the 18-connected case, so I'm just letting it fall through to the 
		//6-connected case. This means 'h' will be bigger than it should be, resulting in a faster path which may not 
		//actually be the shortest one. If you have a correct heuristic for the 18-connected case then please let me know.

		return SixConnectedCost(a,b);
	}

	float TwentySixConnectedCost(const Vector3DInt16& a, const Vector3DInt16& b)
	{
		//Can't say I'm certain about this heuristic - if anyone has
		//a better idea of what it should be then please let me know.
		uint16_t array[3];
		array[0] = abs(a.getX() - b.getX());
		array[1] = abs(a.getY() - b.getY());
		array[2] = abs(a.getZ() - b.getZ());

		std::sort(&array[0], &array[3]);

		uint16_t cornerSteps = array[0];
		uint16_t edgeSteps = array[1] - array[0];
		uint16_t faceSteps = array[2] - array[1];

		return cornerSteps * 1.73205081f + edgeSteps * 1.41421356f + faceSteps * 1.0f;
	}

	float computeH(const Vector3DInt16& a, const Vector3DInt16& b, Connectivity eConnectivity)
	{
		float hVal;
			
		switch(eConnectivity)
		{
		case TwentySixConnected:
			hVal = TwentySixConnectedCost(a, b);
			break;
		case EighteenConnected:
			hVal = EighteenConnectedCost(a, b);
			break;
		case SixConnected:				
			hVal = SixConnectedCost(a, b);
			break;
		default:
			assert(false); //Invalid case.
		}

		std::hash<uint32_t> uint32Hash;

		uint32_t hashValX = uint32Hash(a.getX());
		uint32_t hashValY = uint32Hash(a.getY());
		uint32_t hashValZ = uint32Hash(a.getZ());

		uint32_t hashVal = hashValX ^ hashValY ^ hashValZ;

		hashVal &= 0x0000FFFF;

		float fHash = hashVal / 1000000.0f;

		hVal += fHash;

		return hVal;
	}
}