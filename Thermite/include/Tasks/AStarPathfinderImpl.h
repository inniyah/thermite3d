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

#ifndef __PolyVox_AStarPathfinderImpl_H__
#define __PolyVox_AStarPathfinderImpl_H__

#include "Vector.h"

#include <set>
#include <vector>

namespace PolyVox
{
	class OpenNodesContainer;
	class ClosedNodesContainer;
	class ThermiteGameLogic;

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

		bool operator<(const Node& rhs) const
		{
			if (position.getX() < rhs.position.getX())
				return true;
			if (rhs.position.getX() < position.getX())
				return false;

			if (position.getY() < rhs.position.getY())
				return true;
			if (rhs.position.getY() < position.getY())
				return false;

			if (position.getZ() < rhs.position.getZ())
				return true;
			if (rhs.position.getZ() < position.getZ())
				return false;

			return false;
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

			//return 1.0f;

			float h = abs(position.getX()-endPos.getX()) + abs(position.getY()-endPos.getY());
			return h;

			/*Vector3DFloat endToCurrent = static_cast<Vector3DFloat>(position - endPos);
			Vector3DFloat endToStart = static_cast<Vector3DFloat>(startPos - endPos);

			float angle = endToCurrent.angleTo(endToStart);

			angle *= 0.01f;

			return h + angle;*/
		}

		static PolyVox::Vector3DInt16 startPos;
		static PolyVox::Vector3DInt16 endPos;
	};

	class AllNodesContainer
	{
	public:
		typedef std::set<Node>::iterator iterator;

		AllNodesContainer() {}

		iterator getNode(int x, int y, int z)
		{
			Node nodeToFind(x, y, z);

			return nodes.insert(nodeToFind).first;
		}

	private:
		std::set<Node> nodes;
	};

	class AllNodesContainerIteratorComparator
	{
	public:
		bool operator() (const AllNodesContainer::iterator& lhs, const  AllNodesContainer::iterator& rhs) const
		{
			return (&(*lhs)) < (&(*rhs));
		}
	};

	class NodeSort
	{
	public:
		bool operator() (const AllNodesContainer::iterator& lhs, const AllNodesContainer::iterator& rhs) const
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
		typedef std::set<AllNodesContainer::iterator, AllNodesContainerIteratorComparator>::iterator iterator;

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
		std::set<AllNodesContainer::iterator, AllNodesContainerIteratorComparator> closed;
	};


	//bool operator<(const AllNodesContainer::iterator& lhs, const  AllNodesContainer::iterator& rhs);
}

#endif //__PolyVox_AStarPathfinderImpl_H__