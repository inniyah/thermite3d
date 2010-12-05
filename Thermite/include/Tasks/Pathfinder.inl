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
	/*template <typename VoxelType>
	static Vector3DInt16 Pathfinder<VoxelType>::m_pFaces[6] =
	{
		Vector3DInt16(0, 0, -1),
		Vector3DInt16(0, 0, +1),
		Vector3DInt16(0, -1, 0),
		Vector3DInt16(0, +1, 0),
		Vector3DInt16(-1, 0, 0),
		Vector3DInt16(+1, 0, 0)
	}*/

	template <typename VoxelType>
	Pathfinder<VoxelType>::Pathfinder(Volume<VoxelType>* volData, const Vector3DInt16& v3dStart, const Vector3DInt16& v3dEnd, std::list<Vector3DInt16>* listResult)
		:m_volData(volData)
		,m_v3dStart(v3dStart)
		,m_v3dEnd(v3dEnd)
		,m_listResult(listResult)
	{
		m_pFaces[0].setElements(0, 0, -1);
		m_pFaces[1].setElements(0, 0, +1);
		m_pFaces[2].setElements(0, -1, 0);
		m_pFaces[3].setElements(0, +1, 0);
		m_pFaces[4].setElements(-1, 0, 0);
		m_pFaces[5].setElements(+1, 0, 0);

		for(uint32_t ct = 0; ct < 6; ct++)
		{
			m_pIndices[ct] = ct;
		}
	}

	template <typename VoxelType>
	void Pathfinder<VoxelType>::execute()
	{
		//TODO - clear containers.

		AllNodesContainer::iterator startNode = allNodes.getNode(m_v3dStart.getX(), m_v3dStart.getY(), m_v3dStart.getZ());
		AllNodesContainer::iterator endNode = allNodes.getNode(m_v3dEnd.getX(), m_v3dEnd.getY(), m_v3dEnd.getZ());

		Node::startPos = startNode->position;
		Node::endPos = endNode->position;

		Node* temp = const_cast<Node*>(&(*startNode));
		temp->gVal = 0;

		openNodes.insert(startNode);
		//srand(123456);

		while(openNodes.getFirst() != endNode)
		{
			//Move the first node from open to closed.
			current = openNodes.getFirst();
			openNodes.removeFirst();
			closedNodes.insert(current);

			//random_shuffle(&m_pIndices[0], &m_pIndices[6]);

			//Process the neighbours
			processNeighbour(current->position + m_pFaces[m_pIndices[0]], current->gVal + 1.0f);
			processNeighbour(current->position + m_pFaces[m_pIndices[1]], current->gVal + 1.0f);
			processNeighbour(current->position + m_pFaces[m_pIndices[2]], current->gVal + 1.0f);
			processNeighbour(current->position + m_pFaces[m_pIndices[3]], current->gVal + 1.0f);
			processNeighbour(current->position + m_pFaces[m_pIndices[4]], current->gVal + 1.0f);
			processNeighbour(current->position + m_pFaces[m_pIndices[5]], current->gVal + 1.0f);			
		}

		Node* n = const_cast<Node*>(&(*endNode));
		while(n != 0)
		{
			m_listResult->push_front(n->position);
			n = n->parent;
		}
	}

	template <typename VoxelType>
	void Pathfinder<VoxelType>::processNeighbour(const Vector3DInt16& neighbourPos, float neighbourGVal)
	{
		bool bIsVoxelValidForPath = isVoxelValidForPath(m_volData, neighbourPos);
		if(!bIsVoxelValidForPath)
		{
			return;
		}

		float cost = neighbourGVal;

		AllNodesContainer::iterator neighbour = allNodes.getNode(neighbourPos.getX(), neighbourPos.getY(), neighbourPos.getZ());

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
	bool Pathfinder<VoxelType>::isVoxelValidForPath(const Volume<VoxelType>* volData, const Vector3DInt16& v3dPos)
	{
		//For tanks wars, nodes are only valid if they lie on the 2D plane.
		if(v3dPos.getY() != m_v3dStart.getY())
		{
			return false;
		}

		if(m_volData->getEnclosingRegion().containsPoint(v3dPos) == false)
		{
			return false;
		}

		Material8 voxel = m_volData->getVoxelAt(static_cast<Vector3DUint16>(v3dPos));
		if(voxel.getMaterial() > 0)
		{
			return false;
		}

		return true;
	}
}