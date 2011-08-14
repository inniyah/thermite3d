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

#include "AmbientOcclusionTask.h"

#include "PolyVoxCore/Density.h"
#include "PolyVoxCore/LowPassFilter.h"
#include "PolyVoxCore/Material.h"

#include "PolyVoxCore/AmbientOcclusionCalculator.h"

#include <QMutex>

using namespace PolyVox;

namespace Thermite
{
	PolyVox::RawVolume<PolyVox::Density8>* AmbientOcclusionTask::mThresholdVolume = 0;
	PolyVox::RawVolume<PolyVox::Density8>* AmbientOcclusionTask::mBlurredVolume = 0;

	AmbientOcclusionTask::AmbientOcclusionTask(PolyVox::SimpleVolume<PolyVox::Material16>* volume, PolyVox::Array<3, uint8_t>* ambientOcclusionVolume, PolyVox::Region regToProcess, uint32_t uTimeStamp, float rayLength)
		:m_regToProcess(regToProcess)
		,mAmbientOcclusionVolume(ambientOcclusionVolume)
		,m_uTimeStamp(uTimeStamp)
		,mVolume(volume)
		,mRayLength(rayLength)
	{
		if(mThresholdVolume == 0)
		{
			mThresholdVolume = new PolyVox::RawVolume<PolyVox::Density8>(Region(0,0,0,127,127,127));
		}

		if(mBlurredVolume == 0)
		{
			mBlurredVolume = new PolyVox::RawVolume<PolyVox::Density8>(Region(0,0,0,127,127,127));
		}
	}
	
	void AmbientOcclusionTask::run(void)
	{	
		uint8_t uNoOfSamplesPerOutputElement = 0; //Max off 255 for max quality.
		/*AmbientOcclusionCalculator<SimpleVolume, Material16> ambientOcclusionCalculator(mVolume, mAmbientOcclusionVolume, m_regToProcess, mRayLength, uNoOfSamplesPerOutputElement);
		ambientOcclusionCalculator.execute();*/

		for(uint32_t z = 0; z < mThresholdVolume->getDepth(); z++)
		{
			for(uint32_t y = 0; y < mThresholdVolume->getHeight(); y++)
			{
				for(uint32_t x = 0; x < mThresholdVolume->getWidth(); x++)
				{
					if(mVolume->getVoxelAt(x,y,z).getMaterial() == 0)
					{
						mThresholdVolume->setVoxelAt(x,y,z,255);
					}
					else
					{
						mThresholdVolume->setVoxelAt(x,y,z,0);
					}
				}
			}
		}

		LowPassFilter<RawVolume, RawVolume, Density8> pass1(mThresholdVolume, Region(0,0,0,127,127,127), mBlurredVolume, Region(0,0,0,127,127,127), 5);
		/*LowPassFilter<RawVolume, RawVolume, Density8> pass2(mBlurredVolume, Region(0,0,0,127,127,127), mThresholdVolume, Region(0,0,0,127,127,127));
		pass1.execute();
		pass2.execute();*/
		//pass1.executeSAT();

		for(uint32_t z = 0; z < mAmbientOcclusionVolume->getDimension(2); z++)
		{
			for(uint32_t y = 0; y < mAmbientOcclusionVolume->getDimension(1); y++)
			{
				for(uint32_t x = 0; x < mAmbientOcclusionVolume->getDimension(0); x++)
				{
					(*mAmbientOcclusionVolume)[z][y][x] = mBlurredVolume->getVoxelAt(x,y,z).getDensity();
					//(*mAmbientOcclusionVolume)[z][y][x] = 128;
				}
			}
		}

		emit finished(this);

		return;

#ifdef BLAH
		/*for(int32_t oz = mVolume->getEnclosingRegion().getLowerCorner().getZ(); oz < mVolume->getEnclosingRegion().getUpperCorner().getZ(); oz++)
		{
			for(int32_t oy = mVolume->getEnclosingRegion().getLowerCorner().getY(); oy < mVolume->getEnclosingRegion().getUpperCorner().getY(); oy++)
			{
				for(int32_t ox = mVolume->getEnclosingRegion().getLowerCorner().getX(); ox < mVolume->getEnclosingRegion().getUpperCorner().getX(); ox++)
				{
					(*mAmbientOcclusionVolume)[oz][oy][ox] = 0; //FIXME - might need offset here if volume and ambient volume don't both start at (0,0,0)

					int32_t border = 1;
					for(int32_t iz = oz - border; iz <= oz + border; iz++)
					{
						for(int32_t iy = oy - border; iy <= oy + border; iy++)
						{
							for(int32_t ix = ox - border; ix <= ox + border; ix++)
							{
								if(mVolume->getVoxelAt(ix,iy,iz).getMaterial() == 0) //FIXME - should use density not material
								{
									(*mAmbientOcclusionVolume)[oz][oy][ox] += 9; //FIXME - should not be 9
								}
							}
						}
					}
				}
			}
		}*/

		int32_t border = 5;

		Array<3, uint32_t> SAT(ArraySizes(mVolume->getWidth() + border * 2)(mVolume->getHeight() + border * 2)(mVolume->getDepth() + border * 2));

		//int32_t worldX = mVolume->getEnclosingRegion().getLowerCorner().getX() - border;
		//int32_t worldY = mVolume->getEnclosingRegion().getLowerCorner().getY() - border;
		//int32_t worldZ = mVolume->getEnclosingRegion().getLowerCorner().getZ() - border;

		for(int32_t satZ = 0, worldZ = mVolume->getEnclosingRegion().getLowerCorner().getZ() - border; satZ < SAT.getDimension(2); satZ++, worldZ++)
		{
			for(int32_t satY = 0, worldY = mVolume->getEnclosingRegion().getLowerCorner().getY() - border; satY < SAT.getDimension(1); satY++, worldY++)
			{
				SAT[satZ][satY][0] = mVolume->getVoxelAt(mVolume->getEnclosingRegion().getLowerCorner().getX() - border,worldY,worldZ).getMaterial() == 0 ? 0 : 1;
				for(int32_t satX = 1, worldX = mVolume->getEnclosingRegion().getLowerCorner().getX() - border + 1; satX < SAT.getDimension(0); satX++, worldX++)
				{
					uint32_t isSolid = mVolume->getVoxelAt(worldX,worldY,worldZ).getMaterial() == 0 ? 0 : 1;
					SAT[satZ][satY][satX] = SAT[satZ][satY][satX-1] + isSolid;
				}
			}
		}


		/*worldX = mVolume->getEnclosingRegion().getLowerCorner().getX() - border;
		worldY = mVolume->getEnclosingRegion().getLowerCorner().getY() - border;
		worldZ = mVolume->getEnclosingRegion().getLowerCorner().getZ() - border;*/

		for(int32_t satZ = 0; satZ < SAT.getDimension(2); satZ++)
		{
			for(int32_t satY = 1; satY < SAT.getDimension(1); satY++)
			{
				for(int32_t satX = 0; satX < SAT.getDimension(0); satX++)
				{
					SAT[satZ][satY][satX] += SAT[satZ][satY-1][satX];
				}
			}
		}

		for(int32_t satZ = 1; satZ < SAT.getDimension(2); satZ++)
		{
			for(int32_t satY = 0; satY < SAT.getDimension(1); satY++)
			{
				for(int32_t satX = 0; satX < SAT.getDimension(0); satX++)
				{
					SAT[satZ][satY][satX] += SAT[satZ-1][satY][satX];
				}
			}
		}

		for(int32_t ambVolZ = 0; ambVolZ < mAmbientOcclusionVolume->getDimension(2); ambVolZ++)
		{
			for(int32_t ambVolY = 0; ambVolY < mAmbientOcclusionVolume->getDimension(1); ambVolY++)
			{
				for(int32_t ambVolX = 0; ambVolX < mAmbientOcclusionVolume->getDimension(0); ambVolX++)
				{
					int32_t satLowerX = ambVolX;
					int32_t satLowerY = ambVolY;
					int32_t satLowerZ = ambVolZ;

					int32_t satUpperX = ambVolX + border * 2;
					int32_t satUpperY = ambVolY + border * 2;
					int32_t satUpperZ = ambVolZ + border * 2;

					int32_t a = SAT[satLowerZ][satLowerY][satLowerX];
					int32_t b = SAT[satLowerZ][satLowerY][satUpperX];
					int32_t c = SAT[satLowerZ][satUpperY][satLowerX];
					int32_t d = SAT[satLowerZ][satUpperY][satUpperX];
					int32_t e = SAT[satUpperZ][satLowerY][satLowerX];
					int32_t f = SAT[satUpperZ][satLowerY][satUpperX];
					int32_t g = SAT[satUpperZ][satUpperY][satLowerX];
					int32_t h = SAT[satUpperZ][satUpperY][satUpperX];

					float noSolid = h+c-d-g-f-a+b+e;
					float maxSolid = border * 2/* + 1*/;
					maxSolid = maxSolid * maxSolid * maxSolid;

					float percentSolid = noSolid / maxSolid;
					float percentEmpty = 1.0f - percentSolid;

					(*mAmbientOcclusionVolume)[ambVolZ][ambVolY][ambVolX] = 255 * percentEmpty;

					//(*mAmbientOcclusionVolume)[ambVolZ][ambVolY][ambVolX] = 255 - ((h+c-d-g-f-a+b+e) * 19); //FIXME - should not be 9
				}
			}
		}





		/*for(int32_t oz = mVolume->getEnclosingRegion().getLowerCorner().getZ(); oz < mVolume->getEnclosingRegion().getUpperCorner().getZ(); oz++)
		{
			for(int32_t oy = mVolume->getEnclosingRegion().getLowerCorner().getY(); oy < mVolume->getEnclosingRegion().getUpperCorner().getY(); oy++)
			{
				for(int32_t ox = mVolume->getEnclosingRegion().getLowerCorner().getX(); ox < mVolume->getEnclosingRegion().getUpperCorner().getX(); ox++)
				{
					(*mAmbientOcclusionVolume)[oz][oy][ox] = 0; //FIXME - might need offset here if volume and ambient volume don't both start at (0,0,0)

					
					for(int32_t iz = oz - border; iz <= oz + border; iz++)
					{
						for(int32_t iy = oy - border; iy <= oy + border; iy++)
						{
							for(int32_t ix = ox - border; ix <= ox + border; ix++)
							{
								if(mVolume->getVoxelAt(ix,iy,iz).getMaterial() == 0) //FIXME - should use density not material
								{
									(*mAmbientOcclusionVolume)[oz][oy][ox] += 9; //FIXME - should not be 9
								}
							}
						}
					}
				}
			}
		}*/

		emit finished(this);

#endif
	}
}
