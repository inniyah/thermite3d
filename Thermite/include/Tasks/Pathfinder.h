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

#ifndef __PolyVox_Pathfinder_H__
#define __PolyVox_Pathfinder_H__

#include "Array.h"
#include "AStarPathfinderImpl.h"
#include "PolyVoxForwardDeclarations.h"
#include "Volume.h"
#include "VolumeSampler.h"

#include "PolyVoxImpl/TypeDef.h"

#include <functional>

namespace PolyVox
{
	extern const Vector3DInt16 arrayPathfinderFaces[6];
	extern const Vector3DInt16 arrayPathfinderEdges[12];
	extern const Vector3DInt16 arrayPathfinderCorners[8];

	template <typename VoxelType>
	bool aStarDefaultVoxelValidator(const Volume<VoxelType>* volData, const Vector3DInt16& v3dPos);

	template <typename VoxelType>
	class Pathfinder
	{
	public:
		Pathfinder
		(
			Volume<VoxelType>* volData,
			const Vector3DInt16& v3dStart,
			const Vector3DInt16& v3dEnd,
			std::list<Vector3DInt16>* listResult,
			uint32_t uMaxNoOfNodes = 10000,
			Connectivity connectivity = TwentySixConnected,
			std::function<bool (const Volume<VoxelType>*, const Vector3DInt16&)> funcIsVoxelValidForPath = &aStarDefaultVoxelValidator<VoxelType>,
			std::function<void (float)> funcProgressCallback = 0
		);

		void execute();

	private:
		void processNeighbour(const Vector3DInt16& neighbourPos, float neighbourGVal);

		//The volume data and a sampler to access it.
		Volume<VoxelType>* m_volData;

		Vector3DInt16 m_v3dStart;
		Vector3DInt16 m_v3dEnd;

		//The resulting path
		std::list<Vector3DInt16>* m_listResult;

		//Node containers
		AllNodesContainer allNodes;
		OpenNodesContainer openNodes;
		ClosedNodesContainer closedNodes;

		//The current node
		AllNodesContainer::iterator current;

		//The requied connectivity
		Connectivity m_eConnectivity;

		//Max number of nodes to examine
		uint32_t m_uMaxNoOfNodes;

		//Used to determine whether a given voxel is valid.
		std::function<bool (const Volume<VoxelType>*, const Vector3DInt16&)> m_funcIsVoxelValidForPath;

		//Progress callback
		std::function<void (float)> m_funcProgressCallback;
		float m_fProgress;
	};
}

#include "Pathfinder.inl"

#endif //__PolyVox_Pathfinder_H__
