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

#include "Pathfinder.h"

namespace Thermite
{
	template <typename VoxelType>
	class TankWarsVoxelValidator
	{
	public:
		TankWarsVoxelValidator(int16_t iHeight)
			:m_iHeight(iHeight)
		{
		}

		bool operator() (const PolyVox::Volume<VoxelType>* volData, const PolyVox::Vector3DInt16& v3dPos)
		{
			//For tanks wars, nodes are only valid if they lie on the 2D plane.
			if(v3dPos.getY() != m_iHeight)
			{
				return false;
			}

			//Don't go off the edge and leave a border
			if(volData->getEnclosingRegion().containsPoint(v3dPos, 2) == false)
			{
				return false;
			}

			//Ensure the space is big enough for a tank
			const int16_t tankSize = 3;
			const int16_t y = v3dPos.getY();
			for(int16_t z = v3dPos.getZ() - tankSize; z <= v3dPos.getZ() + tankSize; z++)
			{
				for(int16_t x = v3dPos.getX() - tankSize; x <= v3dPos.getX() + tankSize; x++)
				{
					Material8 voxel = volData->getVoxelAt(x,y,z);
					if(voxel.getMaterial() > 0)
					{
						return false;
					}
				}
			}

			return true;
		}

	private:
		int16_t m_iHeight;
	};

	class FindPathTask : public Task
	{
		Q_OBJECT
	public:
		FindPathTask(PolyVox::Volume<PolyVox::Material8>* polyVoxVolume, QVector3D start, QVector3D end, Volume* thermiteVolume);

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
