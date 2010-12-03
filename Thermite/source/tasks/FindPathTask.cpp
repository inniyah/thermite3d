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

#include "FindPathTask.h"

#include "Material.h"
#include "ThermiteGameLogic.h"

using namespace PolyVox;

namespace Thermite
{
	class NodeSort
	{
	public:
		bool operator() (const AllNodesContainer::iterator lhs, const AllNodesContainer::iterator rhs) const
		{
			return lhs->g() + lhs->h() > rhs->g() + rhs->h();
		}
	};

	class OpenNodesContainer
	{
	public:
		typedef std::vector<AllNodesContainer::iterator>::iterator iterator;

	public:
		void insert(AllNodesContainer::iterator node)
		{
			open.push_back(node);
			push_heap(open.begin(), open.end(), NodeSort());
		}

		AllNodesContainer::iterator getFirst(void)
		{
			return open[0];
		}

		void removeFirst(void)
		{
			pop_heap(open.begin(), open.end(), NodeSort());
			open.pop_back();
		}

		void remove(iterator iterToRemove)
		{
			open.erase(iterToRemove);
			make_heap(open.begin(), open.end(), NodeSort());
		}

		iterator begin(void)
		{
			return open.begin();
		}

		iterator end(void)
		{
			return open.end();
		}

		iterator find(AllNodesContainer::iterator node)
		{
			std::vector<AllNodesContainer::iterator>::iterator openIter = std::find(open.begin(), open.end(), node);
			return openIter;
		}

	private:
		std::vector<AllNodesContainer::iterator> open;
	};

	class ClosedNodesContainer
	{
	public:
		typedef std::set<AllNodesContainer::iterator>::iterator iterator;

	public:
		void insert(AllNodesContainer::iterator node)
		{
			closed.insert(node);
		}

		void remove(iterator iterToRemove)
		{
			closed.erase(iterToRemove);
		}

		iterator begin(void)
		{
			return closed.begin();
		}

		iterator end(void)
		{
			return closed.end();
		}

		iterator find(AllNodesContainer::iterator node)
		{
			iterator iter = std::find(closed.begin(), closed.end(), node);
			return iter;
		}

	private:
		std::set<AllNodesContainer::iterator> closed;
	};

	bool operator<(const Node& lhs, const Node& rhs)
	{
		if (lhs.position.getX() < rhs.position.getX())
			return true;
		if (rhs.position.getX() < lhs.position.getX())
			return false;

		if (lhs.position.getY() < rhs.position.getY())
			return true;
		if (rhs.position.getY() < lhs.position.getY())
			return false;

		if (lhs.position.getZ() < rhs.position.getZ())
			return true;
		if (rhs.position.getZ() < lhs.position.getZ())
			return false;

		return false;
	}

	bool operator<(const AllNodesContainer::iterator& lhs, const  AllNodesContainer::iterator& rhs)
	{
		return (&(*lhs)) < (&(*rhs));
	}

	Vector3DInt16 Node::endPos = Vector3DInt16(0,0,0);

	FindPathTask::FindPathTask(PolyVox::Volume<PolyVox::Material8>* polyVoxVolume, QVector3D start, QVector3D end, Volume* thermiteVolume)
		:mPolyVoxVolume(polyVoxVolume)
		,mStart(start)
		,mEnd(end)
		,mThermiteVolume(thermiteVolume)
	{
	}

	void FindPathTask::run(void)
	{
		findPath(mStart, mEnd);
		//emit finished(this);
	}

	void FindPathTask::findPath(QVector3D start, QVector3D end)
	{
		int startX = start.x() + 0.5f;
		int startY = start.y() + 0.5f;
		int startZ = start.z() + 0.5f;
		int endX = end.x() + 0.5f;
		int endY = end.y() + 0.5f;
		int endZ = end.z() + 0.5f;

		QVariantList result;

		AllNodesContainer allNodes(start.y() + 0.5f);

		//std::vector<Node*> open;
		OpenNodesContainer openNodes;
		//make_heap(open.begin(), open.end());
		//std::set<Node*> closed;
		ClosedNodesContainer closedNodes;

		AllNodesContainer::iterator startNode = allNodes.getNode(startX,startY,startZ);
		AllNodesContainer::iterator endNode = allNodes.getNode(endX, endY, endZ);

		Node::endPos = endNode->position;

		Node* temp = const_cast<Node*>(&(*startNode));
		temp->gVal = 0;

		openNodes.insert(startNode);

		while(openNodes.getFirst() != endNode)
		{
			AllNodesContainer::iterator current = openNodes.getFirst();
			openNodes.removeFirst();

			int currentX = current->position.getX();
			int currentY = current->position.getY();
			int currentZ = current->position.getZ();

			//closed.insert(current);
			closedNodes.insert(current);

			//Neighbours
			if(currentX < 255)
				processNeighbour(current, allNodes.getNode(currentX + 1, startY, currentZ), openNodes, closedNodes);
			if(currentX > 0)
				processNeighbour(current, allNodes.getNode(currentX - 1, startY, currentZ), openNodes, closedNodes);
			if(currentZ < 255)
				processNeighbour(current, allNodes.getNode(currentX, startY, currentZ + 1), openNodes, closedNodes);
			if(currentZ > 0)
				processNeighbour(current, allNodes.getNode(currentX, startY, currentZ - 1), openNodes, closedNodes);
			
		}

		Node* n = const_cast<Node*>(&(*endNode));
		while(n != 0)
		{
			result.prepend(QVector3D(n->position.getX(), n->position.getY(), n->position.getZ()));
			n = n->parent;
		}

		emit finished(result);
	}

	void FindPathTask::processNeighbour(AllNodesContainer::iterator current, AllNodesContainer::iterator neighbour, OpenNodesContainer& openNodes, ClosedNodesContainer& closedNodes)
	{
		Material8 voxel = mPolyVoxVolume->getVoxelAt(neighbour->position.getX(), neighbour->position.getY(), neighbour->position.getZ());
		int cost = current->gVal + 1.0f;

		if(voxel.getMaterial() > 0)
		{
			cost = 1000000;
		}

		//std::vector<Node*>::iterator openIter = std::find(open.begin(), open.end(), neighbour);
		OpenNodesContainer::iterator openIter = openNodes.find(neighbour);
		if(openIter != openNodes.end())
		{
			if(cost < neighbour->gVal)
			{
				openNodes.remove(openIter);
				openIter = openNodes.end();
			}
		}

		ClosedNodesContainer::iterator closedIter = closedNodes.find(neighbour);
		if(closedIter != closedNodes.end())
		{
			if(cost < neighbour->gVal)
			{
				//Probably shouldn't happen?
				//closed.erase(closedIter);
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
}
