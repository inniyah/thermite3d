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

#include <cfloat> //For numeric_limits

namespace PolyVox
{
	template <typename VoxelType>
	Pathfinder<VoxelType>::Pathfinder
	(
		Volume<VoxelType>* volData,
		const Vector3DInt16& v3dStart,
		const Vector3DInt16& v3dEnd,
		std::list<Vector3DInt16>* listResult,
		uint32_t uMaxNoOfNodes,
		Connectivity connectivity,
		std::function<bool (const Volume<VoxelType>*, const Vector3DInt16&)> funcIsVoxelValidForPath,
		std::function<void (float)> funcProgressCallback
	)
		:m_volData(volData)
		,m_v3dStart(v3dStart)
		,m_v3dEnd(v3dEnd)
		,m_listResult(listResult)
		,m_eConnectivity(connectivity)
		,m_funcIsVoxelValidForPath(funcIsVoxelValidForPath)
		,m_uMaxNoOfNodes(uMaxNoOfNodes)
		,m_funcProgressCallback(funcProgressCallback)
	{
	}

	template <typename VoxelType>
	void Pathfinder<VoxelType>::execute()
	{
		//Clear any existing nodes
		allNodes.clear();
		openNodes.clear();
		closedNodes.clear();

		//Clear the result
		m_listResult->clear();

		AllNodesContainer::iterator startNode = allNodes.insert(Node(m_v3dStart.getX(), m_v3dStart.getY(), m_v3dStart.getZ())).first;
		AllNodesContainer::iterator endNode = allNodes.insert(Node(m_v3dEnd.getX(), m_v3dEnd.getY(), m_v3dEnd.getZ())).first;

		/*Node::startPos = startNode->position;
		Node::endPos = endNode->position;
		Node::m_eConnectivity = m_eConnectivity;*/

		Node* tempStart = const_cast<Node*>(&(*startNode));
		tempStart->gVal = 0;
		tempStart->hVal = computeH(startNode->position, endNode->position, m_eConnectivity);

		Node* tempEnd = const_cast<Node*>(&(*endNode));
		tempEnd->hVal = 0.0f;

		openNodes.insert(startNode);

		float fDistStartToEnd = (endNode->position - startNode->position).length();
		m_fProgress = 0.0f;
		if(m_funcProgressCallback)
		{
			m_funcProgressCallback(m_fProgress);
		}

		while((openNodes.empty() == false) && (openNodes.getFirst() != endNode))
		{
			//Move the first node from open to closed.
			current = openNodes.getFirst();
			openNodes.removeFirst();
			closedNodes.insert(current);

			//Update the user on our progress
			if(m_funcProgressCallback)
			{
				const float fMinProgresIncreament = 0.001f;
				float fDistCurrentToEnd = (endNode->position - current->position).length();
				float fDistNormalised = fDistCurrentToEnd / fDistStartToEnd;
				float fProgress = 1.0f - fDistNormalised;
				if(fProgress >= m_fProgress + fMinProgresIncreament)
				{
					m_fProgress = fProgress;
					m_funcProgressCallback(m_fProgress);
				}
			}

			//The distance from one cell to another connected by face, edge, or corner.
			const float fFaceCost = 1.0f;
			const float fEdgeCost = 1.41421356f; //sqrt(2)
			const float fCornerCost = 1.73205081f; //sqrt(3)

			//Process the neighbours. Note the deliberate lack of 'break' 
			//statements, larger connectivities include smaller ones.
			switch(m_eConnectivity)
			{
			case TwentySixConnected:
				processNeighbour(current->position + arrayPathfinderCorners[0], current->gVal + fCornerCost);
				processNeighbour(current->position + arrayPathfinderCorners[1], current->gVal + fCornerCost);
				processNeighbour(current->position + arrayPathfinderCorners[2], current->gVal + fCornerCost);
				processNeighbour(current->position + arrayPathfinderCorners[3], current->gVal + fCornerCost);
				processNeighbour(current->position + arrayPathfinderCorners[4], current->gVal + fCornerCost);
				processNeighbour(current->position + arrayPathfinderCorners[5], current->gVal + fCornerCost);
				processNeighbour(current->position + arrayPathfinderCorners[6], current->gVal + fCornerCost);
				processNeighbour(current->position + arrayPathfinderCorners[7], current->gVal + fCornerCost);

			case EighteenConnected:
				processNeighbour(current->position + arrayPathfinderEdges[ 0], current->gVal + fEdgeCost);
				processNeighbour(current->position + arrayPathfinderEdges[ 1], current->gVal + fEdgeCost);
				processNeighbour(current->position + arrayPathfinderEdges[ 2], current->gVal + fEdgeCost);
				processNeighbour(current->position + arrayPathfinderEdges[ 3], current->gVal + fEdgeCost);
				processNeighbour(current->position + arrayPathfinderEdges[ 4], current->gVal + fEdgeCost);
				processNeighbour(current->position + arrayPathfinderEdges[ 5], current->gVal + fEdgeCost);
				processNeighbour(current->position + arrayPathfinderEdges[ 6], current->gVal + fEdgeCost);
				processNeighbour(current->position + arrayPathfinderEdges[ 7], current->gVal + fEdgeCost);
				processNeighbour(current->position + arrayPathfinderEdges[ 8], current->gVal + fEdgeCost);
				processNeighbour(current->position + arrayPathfinderEdges[ 9], current->gVal + fEdgeCost);
				processNeighbour(current->position + arrayPathfinderEdges[10], current->gVal + fEdgeCost);
				processNeighbour(current->position + arrayPathfinderEdges[11], current->gVal + fEdgeCost);

			case SixConnected:
				processNeighbour(current->position + arrayPathfinderFaces[0], current->gVal + fFaceCost);
				processNeighbour(current->position + arrayPathfinderFaces[1], current->gVal + fFaceCost);
				processNeighbour(current->position + arrayPathfinderFaces[2], current->gVal + fFaceCost);
				processNeighbour(current->position + arrayPathfinderFaces[3], current->gVal + fFaceCost);
				processNeighbour(current->position + arrayPathfinderFaces[4], current->gVal + fFaceCost);
				processNeighbour(current->position + arrayPathfinderFaces[5], current->gVal + fFaceCost);
			}

			if(allNodes.size() > m_uMaxNoOfNodes)
			{
				//We've reached the specified maximum number
				//of nodes. Just give up on the search.
				break;
			}
		}

		if((openNodes.empty()) || (openNodes.getFirst() != endNode))
		{
			//In this case we failed to find a valid path.
			throw runtime_error("No path found");
		}
		else
		{
			Node* n = const_cast<Node*>(&(*endNode));
			while(n != 0)
			{
				m_listResult->push_front(n->position);
				n = n->parent;
			}
		}

		if(m_funcProgressCallback)
		{
			m_funcProgressCallback(1.0f);
		}
	}

	template <typename VoxelType>
	void Pathfinder<VoxelType>::processNeighbour(const Vector3DInt16& neighbourPos, float neighbourGVal)
	{
		bool bIsVoxelValidForPath = m_funcIsVoxelValidForPath(m_volData, neighbourPos);
		if(!bIsVoxelValidForPath)
		{
			return;
		}

		float cost = neighbourGVal;

		std::pair<AllNodesContainer::iterator, bool> insertResult = allNodes.insert(Node(neighbourPos.getX(), neighbourPos.getY(), neighbourPos.getZ()));
		AllNodesContainer::iterator neighbour = insertResult.first;

		if(insertResult.second == true) //New node, compute h.
		{
			Node* tempNeighbour = const_cast<Node*>(&(*neighbour));
			tempNeighbour -> hVal = computeH(neighbour->position, m_v3dEnd, m_eConnectivity);
		}

		OpenNodesContainer::iterator openIter = openNodes.find(neighbour);
		if(openIter != openNodes.end())
		{
			if(cost < neighbour->gVal)
			{
				openNodes.remove(openIter);
				openIter = openNodes.end();
			}
		}

		//TODO - Nodes could keep track of if they are in open or closed? And a pointer to where they are?
		ClosedNodesContainer::iterator closedIter = closedNodes.find(neighbour);
		if(closedIter != closedNodes.end())
		{
			if(cost < neighbour->gVal)
			{
				//Probably shouldn't happen?
				closedNodes.remove(closedIter);
				closedIter = closedNodes.end();
			}
		}

		if((openIter == openNodes.end()) && (closedIter == closedNodes.end()))
		{
			Node* temp = const_cast<Node*>(&(*neighbour));

			temp->gVal = cost;

			openNodes.insert(neighbour);

			temp->parent = const_cast<Node*>(&(*current));
		}
	}

	template <typename VoxelType>
	bool aStarDefaultVoxelValidator(const Volume<VoxelType>* volData, const Vector3DInt16& v3dPos)
	{
		//Voxels are considered valid candidates for the path if they are inside the volume...
		if(volData->getEnclosingRegion().containsPoint(v3dPos) == false)
		{
			return false;
		}

		//and if their density is below the threshold.
		Material8 voxel = volData->getVoxelAt(static_cast<Vector3DUint16>(v3dPos));
		if(voxel.getDensity() >= Material8::getThreshold())
		{
			return false;
		}

		return true;
	}
}