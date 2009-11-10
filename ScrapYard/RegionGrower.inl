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

template <typename VoxelType>
	void Volume<VoxelType>::regionGrow(boost::uint16_t xStart, boost::uint16_t yStart, boost::uint16_t zStart, VoxelType value)
	{
		//FIXME - introduce integrer 'isInVolume' function
		if((xStart > POLYVOX_VOLUME_SIDE_LENGTH-1) || (yStart > POLYVOX_VOLUME_SIDE_LENGTH-1) || (zStart > POLYVOX_VOLUME_SIDE_LENGTH-1)
			|| (xStart < 0) || (yStart < 0) || (zStart < 0))
		{
            //FIXME - error message..
			return;
		}

		VolumeIterator<VoxelType> volIter(*this);
		const VoxelType uSeedValue = volIter.getVoxelAt(xStart,yStart,zStart);

		if(value == uSeedValue)
		{
			return; //FIXME - Error message? Exception?
		}

		std::queue<Vector3DUint32> seeds;
		seeds.push(Vector3DUint32(xStart,yStart,zStart));

		while(!seeds.empty())
		{
			Vector3DUint32 currentSeed = seeds.front();
			seeds.pop();

			//std::cout << "x = " << currentSeed.x << " y = " << currentSeed.y << " z = " << currentSeed.z << std::endl;

			//FIXME - introduce 'safe' function which tests this?
			if((currentSeed.x() > POLYVOX_VOLUME_SIDE_LENGTH-2) || (currentSeed.y() > POLYVOX_VOLUME_SIDE_LENGTH-2) || (currentSeed.z() > POLYVOX_VOLUME_SIDE_LENGTH-2)
				|| (currentSeed.x() < 1) || (currentSeed.y() < 1) || (currentSeed.z() < 1))
			{
				continue;
			}

			if(volIter.getVoxelAt(currentSeed.x(), currentSeed.y(), currentSeed.z()+1) == uSeedValue)
			{
				volIter.setVoxelAt(currentSeed.x(), currentSeed.y(), currentSeed.z()+1, value);
				seeds.push(Vector3DUint32(currentSeed.x(), currentSeed.y(), currentSeed.z()+1));
			}

			if(volIter.getVoxelAt(currentSeed.x(), currentSeed.y(), currentSeed.z()-1) == uSeedValue)
			{
				volIter.setVoxelAt(currentSeed.x(), currentSeed.y(), currentSeed.z()-1, value);
				seeds.push(Vector3DUint32(currentSeed.x(), currentSeed.y(), currentSeed.z()-1));
			}

			if(volIter.getVoxelAt(currentSeed.x(), currentSeed.y()+1, currentSeed.z()) == uSeedValue)
			{
				volIter.setVoxelAt(currentSeed.x(), currentSeed.y()+1, currentSeed.z(), value);
				seeds.push(Vector3DUint32(currentSeed.x(), currentSeed.y()+1, currentSeed.z()));
			}

			if(volIter.getVoxelAt(currentSeed.x(), currentSeed.y()-1, currentSeed.z()) == uSeedValue)
			{
				volIter.setVoxelAt(currentSeed.x(), currentSeed.y()-1, currentSeed.z(), value);
				seeds.push(Vector3DUint32(currentSeed.x(), currentSeed.y()-1, currentSeed.z()));
			}

			if(volIter.getVoxelAt(currentSeed.x()+1, currentSeed.y(), currentSeed.z()) == uSeedValue)
			{
				volIter.setVoxelAt(currentSeed.x()+1, currentSeed.y(), currentSeed.z(), value);
				seeds.push(Vector3DUint32(currentSeed.x()+1, currentSeed.y(), currentSeed.z()));
			}

			if(volIter.getVoxelAt(currentSeed.x()-1, currentSeed.y(), currentSeed.z()) == uSeedValue)
			{
				volIter.setVoxelAt(currentSeed.x()-1, currentSeed.y(), currentSeed.z(), value);
				seeds.push(Vector3DUint32(currentSeed.x()-1, currentSeed.y(), currentSeed.z()));
			}
		}
	}