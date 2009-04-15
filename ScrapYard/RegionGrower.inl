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