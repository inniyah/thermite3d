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

Vector3DInt16 Node::startPos = Vector3DInt16(0,0,0);
Vector3DInt16 Node::endPos = Vector3DInt16(0,0,0);

namespace Thermite
{
	FindPathTask::FindPathTask(PolyVox::Volume<PolyVox::Material8>* polyVoxVolume, QVector3D start, QVector3D end, Volume* thermiteVolume)
		:mPolyVoxVolume(polyVoxVolume)
		,mStart(start)
		,mEnd(end)
		,mThermiteVolume(thermiteVolume)
	{
	}

	void FindPathTask::run(void)
	{
		Vector3DInt16 start(mStart.x() + 0.5, mStart.y() + 0.5, mStart.z() + 0.5);
		Vector3DInt16 end(mEnd.x() + 0.5, mEnd.y() + 0.5, mEnd.z() + 0.5);

		list<Vector3DInt16> path;
		Pathfinder<Material8> pathfinder(mPolyVoxVolume, start, end, &path);
		pathfinder.execute();

		QVariantList variantPath;

		for(list<Vector3DInt16>::iterator iter = path.begin(); iter != path.end(); iter++)
		{
			variantPath.append(QVector3D(iter->getX(), iter->getY(), iter->getZ()));
		}

		emit finished(variantPath);
	}
}
