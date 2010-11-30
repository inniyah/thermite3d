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
	Node* Node::startNode = 0;
	Node* Node::endNode = 0;

	class NodeSort
	{
	public:
		bool operator() (const Node* lhs, const Node* rhs) const
		{
			return lhs->g() + lhs->h() > rhs->g() + rhs->h();
		}
	};

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
		int startZ = start.z() + 0.5f;
		int endX = end.x() + 0.5f;
		int endZ = end.z() + 0.5f;

		QVariantList result;

		Node* nodes[256][256];
		//std::fill(nodes[0][0], nodes[255][255], 0);
		for(int z = 0; z < 256; z++)
		{
			for(int x = 0; x < 256; x++)
			{
				Node* node = new Node;

				node->position.setX(x);
				node->position.setY(start.y() + 0.5f);
				node->position.setZ(z);
				node->gVal = 1000000;
				node->parent = 0;

				nodes[x][z] = node;
			}
		}

		Node* startNode = nodes[startX][startZ];
		Node* endNode = nodes[endX][endZ];

		Node::startNode = startNode;
		Node::endNode = endNode;

		std::vector<Node*> open;
		//make_heap(open.begin(), open.end());
		std::set<Node*> closed;

		startNode->gVal = 0;

		open.push_back(startNode);
		push_heap(open.begin(), open.end(), NodeSort());

		while(open[0] != endNode)
		{
			Node* current = open[0];
			pop_heap(open.begin(), open.end(), NodeSort());
			open.pop_back();

			int currentX = current->position.getX();
			int currentZ = current->position.getZ();

			//closed.push_back(current);
			closed.insert(current);

			//Plus X
			//Node* plusX = nodes[currentX + 1][currentZ];
			processNeighbour(current, nodes[currentX + 1][currentZ], open, closed);
			processNeighbour(current, nodes[currentX - 1][currentZ], open, closed);
			processNeighbour(current, nodes[currentX][currentZ + 1], open, closed);
			processNeighbour(current, nodes[currentX][currentZ - 1], open, closed);
			
		}

		Node* n = endNode;
		while(n != 0)
		{
			result.prepend(QVector3D(n->position.getX(), n->position.getY(), n->position.getZ()));
			n = n->parent;
		}

		//result.append(QVector3D(128,end.y(),128));
		//result.append(end);

		emit finished(result);

		//return result;
	}

	void FindPathTask::processNeighbour(Node* current, Node* neighbour, std::vector<Node*>& open, std::set<Node*>& closed)
	{
		Material8 voxel = mPolyVoxVolume->getVoxelAt(neighbour->position.getX(), neighbour->position.getY(), neighbour->position.getZ());
		int cost = current->gVal + 1.0f;

		if(voxel.getMaterial() > 0)
		{
			cost = 1000000;
		}

		std::vector<Node*>::iterator openIter = std::find(open.begin(), open.end(), neighbour);
		if(openIter != open.end())
		{
			if(cost < neighbour->gVal)
			{
				open.erase(openIter);
				make_heap(open.begin(), open.end(), NodeSort());
				openIter = open.end();
			}
		}

		std::set<Node*>::iterator closedIter = std::find(closed.begin(), closed.end(), neighbour);
		if(closedIter != closed.end())
		{
			if(cost < neighbour->gVal)
			{
				//Probably shouldn't happen?
				closed.erase(closedIter);
				closedIter = closed.end();
			}
		}

		if((openIter == open.end()) && (closedIter == closed.end()))
		{
			neighbour->gVal = cost;

			open.push_back(neighbour);
			push_heap(open.begin(), open.end(), NodeSort());

			neighbour->parent = current;
		}
	}
}
