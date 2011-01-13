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

#include "Array.h"
#include "Raycast.h"
#include "Volume.h"
#include "VolumeSampler.h"

namespace PolyVox
{
	template <typename VoxelType>
	VolumeResampler<VoxelType>::VolumeResampler(Volume<VoxelType>* volInput, Array<3, uint8_t>* arrayResult, Region region)
		:m_volInput(volInput)
		,m_arrayResult(arrayResult)
		,m_region(region)
		,m_sampVolume(volInput)
	{
		/*assert(m_volInput.getWidth() == m_volOutput.getWidth() * 2);
		assert(m_volInput.getHeight() == m_volOutput.getHeight() * 2);
		assert(m_volInput.getDepth() == m_volOutput.getDepth() * 2);*/
		srand(12345);
	}

	template <typename VoxelType>
	VolumeResampler<VoxelType>::~VolumeResampler()
	{
	}

	template <typename VoxelType>
	void VolumeResampler<VoxelType>::execute(void)
	{
		const int iRatioX = m_volInput->getWidth()  / m_arrayResult->getDimension(0);
		const int iRatioY = m_volInput->getHeight() / m_arrayResult->getDimension(1);
		const int iRatioZ = m_volInput->getDepth()  / m_arrayResult->getDimension(2);
		const int iRatioMax = std::max(std::max(iRatioX, iRatioY), iRatioZ);

		const float fRatioX = iRatioX;
		const float fRatioY = iRatioY;
		const float fRatioZ = iRatioZ;
		const float fRatioMax = iRatioMax;
		const Vector3DFloat v3dRatio(fRatioX, fRatioY, fRatioZ);

		const float fHalfRatioX = fRatioX * 0.5f;
		const float fHalfRatioY = fRatioY * 0.5f;
		const float fHalfRatioZ = fRatioZ * 0.5f;
		const float fHalfRatioMax = fRatioMax * 0.5f;
		const Vector3DFloat v3dHalfRatio(fHalfRatioX, fHalfRatioY, fHalfRatioZ);

		const Vector3DFloat v3dOffset(0.5f,0.5f,0.5f);

		for(uint16_t z = m_region.getLowerCorner().getZ(); z <= m_region.getUpperCorner().getZ(); z += iRatioZ)
		{
			for(uint16_t y = m_region.getLowerCorner().getY(); y <= m_region.getUpperCorner().getY(); y += iRatioY)
			{
				for(uint16_t x = m_region.getLowerCorner().getX(); x <= m_region.getUpperCorner().getX(); x += iRatioX)
				{
					uint16_t uVisibleDirections = 0;

					for(int ct = 0; ct < 256; ct++)
					{
						Vector3DFloat v3dStart(x, y, z);
						//v3dStart += static_cast<Vector3DFloat>(m_region.getLowerCorner()); //Move cast outside loop.
						v3dStart -= v3dOffset;
						v3dStart += v3dHalfRatio;

						const float sqrt_3 = 1.7321f;
						const float fEscapeFromCube = sqrt_3 * (fHalfRatioMax);
						
						Vector3DFloat v3dJitter = randomJitterVector();
						v3dJitter *= v3dRatio;

						Vector3DFloat unitVector = randomUnitVector();
						Vector3DFloat v3dEscapeFromCube = unitVector * fEscapeFromCube;
						Vector3DFloat vectorScaled = unitVector * 16.0f;

						RaycastResult raycastResult;
						Raycast<VoxelType> raycast(m_volInput, v3dStart + v3dJitter + v3dEscapeFromCube, vectorScaled, raycastResult);
						raycast.execute();

						if(raycastResult.foundIntersection == false)
						{
							uVisibleDirections += 1;
						}
					}
					(*m_arrayResult)[z / iRatioZ][y / iRatioY][x / iRatioX] = uVisibleDirections / 1;
				}
			}
		}
	}

	template <typename VoxelType>
	Vector3DFloat VolumeResampler<VoxelType>::randomJitterVector(void)
	{
		float x = ((float)rand()/(float)RAND_MAX) * 2.0f - 1.0f;
		float y = ((float)rand()/(float)RAND_MAX) * 2.0f - 1.0f;
		float z = ((float)rand()/(float)RAND_MAX) * 2.0f - 1.0f;
		Vector3DFloat result(x,y,z);
		return result;
	}

	template <typename VoxelType>
	Vector3DFloat VolumeResampler<VoxelType>::randomUnitVector(void)
	{
		float x = ((float)rand()/(float)RAND_MAX) * 2.0f - 1.0f;
		float y = ((float)rand()/(float)RAND_MAX) * 2.0f - 1.0f;
		float z = ((float)rand()/(float)RAND_MAX) * 2.0f - 1.0f;
		Vector3DFloat result(x,y,z);
		result.normalise();
		return result;
	}
}