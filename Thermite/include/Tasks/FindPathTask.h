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

#ifndef __THERMITE_FIND_PATH_TASK_H__
#define __THERMITE_FIND_PATH_TASK_H__

#include "Task.h"

#include <Material.h>
#include <PolyVoxForwardDeclarations.h>

#include <QVariant>
#include <QVector3D>

#include "Vector.h"

#include "scriptable/Volume.h"

namespace Thermite
{
	class OpenNodesContainer;
	class ClosedNodesContainer;

	struct Node
	{
		Node()
		{
			position.setX(0);
			position.setY(0);
			position.setZ(0);
			gVal = 1000000;
			parent = 0;
		};

		Node(int x, int y, int z)
		{
			position.setX(x);
			position.setY(y);
			position.setZ(z);
			gVal = 1000000;
			parent = 0;
		}

		bool operator==(const Node& rhs) const
		{
			return position == rhs.position;
		}

		bool operator!=(const Node& rhs) const
		{
			return (position == rhs.position) == false;
		}

		PolyVox::Vector3DInt16 position;
		Node* parent;
		float gVal;

		float g(void) const
		{
			return gVal;
		}

		float h(void) const
		{
			return abs(position.getX()-endPos.getX()) + abs(position.getY()-endPos.getY());
		}

		//static Node* startNode;
		//static Node* endNode;
		static PolyVox::Vector3DInt16 endPos;
	};

	bool operator<(const Node& lhs, const Node& rhs);

	class AllNodesContainer
	{
	public:
		typedef std::set<Node>::iterator iterator;

		AllNodesContainer(int yVal)
		{
			/*for(int z = 0; z < 256; z++)
			{
				for(int x = 0; x < 256; x++)
				{

					Node node;

					node.position.setX(x);
					node.position.setY(yVal);
					node.position.setZ(z);
					node.gVal = 1000000;
					node.parent = 0;

					nodes.push_back(node);
				}
			}*/

			//nodes.reserve(1000000);

			mYVal = yVal;
		}

		iterator getNode(int x, int y, int z)
		{
			Node nodeToFind(x, mYVal, z);
			nodeToFind.gVal = 1000000;
			nodeToFind.parent = 0;

			std::set<Node>::iterator iter = std::find(nodes.begin(), nodes.end(), nodeToFind);

			if(iter == nodes.end())
			{
				nodes.insert(nodeToFind);
				iter = std::find(nodes.begin(), nodes.end(), nodeToFind);
			}

			assert(iter != nodes.end());

			return iter;
		}

	private:
		//Node nodes[256][256];
		std::set<Node> nodes;
		int mYVal;
	};

	bool operator<(const AllNodesContainer::iterator& lhs, const  AllNodesContainer::iterator& rhs);

	class ThermiteGameLogic;

	class FindPathTask : public Task
	{
		Q_OBJECT
	public:
		FindPathTask(PolyVox::Volume<PolyVox::Material8>* polyVoxVolume, QVector3D start, QVector3D end, Volume* thermiteVolume);

		void findPath(QVector3D start, QVector3D end);
		void processNeighbour(AllNodesContainer::iterator current, AllNodesContainer::iterator neighbour, OpenNodesContainer& openNodes, ClosedNodesContainer& closedNodes);

		void run(void);

	signals:
		void finished(QVariantList path);

	public:
		PolyVox::Volume<PolyVox::Material8>* mPolyVoxVolume;
		QVector3D mStart;
		QVector3D mEnd;
		Volume* mThermiteVolume;
	};
}

#endif //__THERMITE_FIND_PATH_TASK_H__
