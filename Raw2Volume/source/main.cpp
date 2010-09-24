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

#include "MaterialDensityPair.h"
#include "Volume.h"
#include "Serialization.h"

#include <QCoreApplication>
#include <QFile>
#include <QImage>
#include <QStringList>

#include <fstream>

using namespace PolyVox;
using namespace std;

struct Arguments
{
	Arguments() : outputFilename("output.volume"), materialID(1){}
	QString inputFilename;
	QString outputFilename;
	uint8_t materialID;
};

void printUsage(void)
{
	cout << endl;
	cout << "Thermite Heightmap to Volume Converter" << endl;
	cout << endl;
	cout << "Usage: Heightmap2Volume -i [file] -o [file] -m [0-255]" << endl;
	cout << endl;
	cout << "    -i The name of the input volume. This arguent is required" << endl;
	cout << "    -o The name of the volume file to write to disk." << endl;
	cout << "       This defaults to \"output.volume\"" << endl;
	cout << "    -u The material ID to use for voxels." << endl;
	cout << "       This defaults to '1'" << endl;
	cout << endl;
}

Arguments parseCommandLine(const QStringList& commandLine)
{
	Arguments arguments;

	//Find the input file name
	int dashIndex = commandLine.indexOf("-i");	
	if((dashIndex != -1) && (dashIndex + 1 < commandLine.size()))
	{
		arguments.inputFilename = commandLine.at(dashIndex + 1);
	}
	else
	{
		throw invalid_argument("Invalid input filename");
	}

	//Find the output file name
	dashIndex = commandLine.indexOf("-o");	
	if((dashIndex != -1) && (dashIndex + 1 < commandLine.size()))
	{
		arguments.outputFilename = commandLine.at(dashIndex + 1);
	}

	//Find the underground material ID
	dashIndex = commandLine.indexOf("-m");	
	if((dashIndex != -1) && (dashIndex + 1 < commandLine.size()))
	{
		QString materialAsString = commandLine.at(dashIndex + 1);
		bool bOk = false;
		uint tempMaterialID = materialAsString.toLong(&bOk);
		if((bOk) && (tempMaterialID <= 255))
		{
			arguments.materialID = static_cast<uint8_t>(tempMaterialID);
		}
		else
		{
			cout << "WARNING: Invalid material ID. Using default value instead" << endl;
		}
	}

	return arguments;
}

bool testIfPowerOf2(unsigned int uInput)
{
	if(uInput == 0)
		return false;
	else
		return ((uInput & (uInput-1)) == 0);
}

int main(int argc, char *argv[])
{
	QCoreApplication app(argc, argv);

	Arguments arguments;
	try
	{
		QStringList commandLine = app.arguments();
		arguments = parseCommandLine(commandLine);
	}
	catch(exception& e)
	{
		printUsage();
		return 1;
	}

	/*QImage image(arguments.inputFilename);
	if(image.isNull())
	{
		cout << "ERROR: Failed to open input image " << arguments.inputFilename.toStdString() << endl;
		return 1;
	}

	if((testIfPowerOf2(image.width()) == false) || (testIfPowerOf2(image.height()) == false))
	{
		cout << "ERROR: Invalid image size. Both width and height must be powers of two." << endl;
		return 1;
	}*/

	QFile inputFile(arguments.inputFilename);
	inputFile.open(QIODevice::ReadOnly);
	QDataStream inStream(&inputFile);    // read the data serialized from the file

	int width = 256;
	int height = 256;
	int depth = 256;

	//Create a volume
	Volume<MaterialDensityPair44> volData(width, height, depth);

	//Clear volume to zeros.
	//FIXME - Add function to PolyVox for this.
	for(unsigned int z = 0; z < volData.getDepth(); ++z)
	{
		for(unsigned int y = 0; y < volData.getHeight(); ++y)
		{
			for(unsigned int x = 0; x < volData.getWidth(); ++x)
			{
				float voxelVal;
				//inStream >> voxelVal;
				inStream.readRawData((char*)&voxelVal, 4);

				MaterialDensityPair44 voxel;
				if(voxelVal > 0.0f)
				{
					voxel.setDensity(MaterialDensityPair44::getMaxDensity());
					voxel.setMaterial(1);
				}
				else
				{
					voxel.setDensity(MaterialDensityPair44::getMinDensity());
					voxel.setMaterial(0);
				}
				volData.setVoxelAt(x,y,z,voxel);
			}
		}
		cout << z << endl;
	}

	ofstream file (arguments.outputFilename.toStdString().c_str(), ios::out|ios::binary);
	saveVolumeRle(file, volData);
	file.close();

	//return app.exec();
	return 0;
}