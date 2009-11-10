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

#ifndef __HalfEdgeSurfacePatch_H__
#define __HalfEdgeSurfacePatch_H__

#include <set>
#include <list>

#include "AbstractSurfacePatch.h"
#include "IntegralVector3.h"
#include "SurfaceTypes.h"
#include "VolumeIterator.h"


namespace Ogre
{
	class HalfEdgeSurfacePatch : public AbstractSurfacePatch
	{
	public:
	   HalfEdgeSurfacePatch();
	   ~HalfEdgeSurfacePatch();

	   //This allow users of the class to iterate over its contents.
	   SurfaceEdgeIterator getEdgesBegin(void);
	   SurfaceEdgeIterator getEdgesEnd(void);
	   SurfaceTriangleIterator getTrianglesBegin(void);
	   SurfaceTriangleIterator getTrianglesEnd(void);	   

	   //Users of the class might want these for debugging or info purposes.
	   uint getNoOfEdges(void) const;
	   uint getNoOfTriangles(void) const;
	   uint getNoOfVertices(void) const;

	   bool canRemoveVertexFrom(SurfaceVertexIterator vertexIter, std::list<SurfaceVertexIterator> listConnectedIter, bool isEdge);
	   std::list<SurfaceVertexIterator> findConnectedVertices(SurfaceVertexIterator vertexIter, bool& isEdge);
	   uint decimate(void);
	   void triangulate(std::list<SurfaceVertexIterator> listVertices);
	   bool isPolygonConvex(std::list<SurfaceVertexIterator> listVertices, Vector3 normal);
	   void addTriangle(const SurfaceVertex& v0,const SurfaceVertex& v1,const SurfaceVertex& v2);

	   void fillVertexAndIndexData(std::vector<SurfaceVertex>& vecVertices, std::vector<ushort>& vecIndices);

	   


	private:		
		std::set<SurfaceTriangle> m_listTriangles;
		std::set<SurfaceEdge> m_listEdges;

		SurfaceEdgeIterator findEdge(const SurfaceVertexIterator& source, const SurfaceVertexIterator& target);
		SurfaceEdgeIterator findOrAddEdge(const SurfaceVertexIterator& source, const SurfaceVertexIterator& target);
		SurfaceVertexIterator findOrAddVertex(const SurfaceVertex& vertex);
	};	
}

#endif /* __HalfEdgeSurfacePatch_H__ */
