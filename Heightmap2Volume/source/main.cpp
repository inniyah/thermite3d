#include "PolyVoxCore/BlockVolume.h"
#include "PolyVoxCore/Utility.h"
#include "PolyVoxUtil/Serialization.h"

#include <QCoreApplication>
#include <QFile>
#include <QImage>
#include <QStringList>

#include <fstream>

using namespace PolyVox;
using namespace std;

struct Arguments
{
	Arguments() : outputFilename("output.volume"), undergroundMaterialID(1), surfaceMaterialID(2), surfaceThickness(5) {}
	QString inputFilename;
	QString outputFilename;
	PolyVox::uint8 undergroundMaterialID;
	PolyVox::uint8 surfaceMaterialID;
	int surfaceThickness;
};

void printUsage(void)
{
	cout << endl;
	cout << "Thermite Heightmap to Volume Converter" << endl;
	cout << endl;
	cout << "Usage: Heightmap2Volume -i [file] -o [file] -u [0-255] -s [0-255] -t [1-10]" << endl;
	cout << endl;
	cout << "    -i The name of the image file. This arguent is required" << endl;
	cout << "       Images dimensions must be a power of two." << endl;
	cout << "       Suppored formats include .bmp, .gif, .jpg/jpeg, .png," << endl;
	cout << "       .pbm, .pgm, .ppm, .tiff, .xbm and .xpm." << endl;
	cout << "    -o The name of the volume file to write to disk." << endl;
	cout << "       This defaults to \"output.volume\"" << endl;
	cout << "    -u The material ID to use for voxels underneath the heightmap." << endl;
	cout << "       This defaults to '1'" << endl;
	cout << "    -s The material ID to use for voxels on the surface of the heightmap." << endl;
	cout << "       This defaults to '2'" << endl;
	cout << "    -t The desired thickness of the surface in voxels." << endl;
	cout << "       This defaults to '5'" << endl;
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
	dashIndex = commandLine.indexOf("-u");	
	if((dashIndex != -1) && (dashIndex + 1 < commandLine.size()))
	{
		QString materialAsString = commandLine.at(dashIndex + 1);
		bool bOk = false;
		uint tempMaterialID = materialAsString.toLong(&bOk);
		if((bOk) && (tempMaterialID <= 255))
		{
			arguments.undergroundMaterialID = static_cast<PolyVox::uint8>(tempMaterialID);
		}
		else
		{
			cout << "WARNING: Invalid underground material ID. Using default value instead" << endl;
		}
	}

	//Find the surface material ID
	dashIndex = commandLine.indexOf("-s");	
	if((dashIndex != -1) && (dashIndex + 1 < commandLine.size()))
	{
		QString materialAsString = commandLine.at(dashIndex + 1);
		bool bOk = false;
		uint tempMaterialID = materialAsString.toLong(&bOk);
		if((bOk) && (tempMaterialID <= 255))
		{
			arguments.surfaceMaterialID = static_cast<PolyVox::uint8>(tempMaterialID);
		}
		else
		{
			cout << "WARNING: Invalid surface material ID. Using default value instead" << endl;
		}
	}

	//Find the surface thickness
	dashIndex = commandLine.indexOf("-t");	
	if((dashIndex != -1) && (dashIndex + 1 < commandLine.size()))
	{
		QString materialAsString = commandLine.at(dashIndex + 1);
		bool bOk = false;
		int tempThickness = materialAsString.toInt(&bOk);
		if((bOk) && (tempThickness <= 255))
		{
			arguments.surfaceThickness = static_cast<PolyVox::uint8>(tempThickness);
		}
		else
		{
			cout << "WARNING: Invalid surface material ID. Using default value instead" << endl;
		}
	}

	return arguments;
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

	QImage image(arguments.inputFilename);
	if(image.isNull())
	{
		cout << "ERROR: Failed to open input image " << arguments.inputFilename.toStdString() << endl;
		return 1;
	}

	if((image.width() != 256) || (image.height() != 256))
	{
		cout << "ERROR: In this version of the converter the input image must be 256x256 pixels. Larger images will be supported soon." << endl;
		return 1;
	}

	//Create a volume
	unsigned int volumeSideLength = 256;
	BlockVolume<uint8> volData(logBase2(volumeSideLength));

	//Create an iterator to access data in the volume
	BlockVolumeIterator<uint8> volIter(volData);

	//Clear volume to zeros.
	//FIXME - Add function to PolyVox for this.
	for(unsigned int z = 0; z < volumeSideLength; ++z)
	{
		for(unsigned int y = 0; y < volumeSideLength; ++y)
		{
			for(unsigned int x = 0; x < volumeSideLength; ++x)
			{
				volIter.setPosition(x,y,z);
				volIter.setVoxel(0);
			}
		}
	}

	for(unsigned int z = 1; z < volumeSideLength-1; ++z)
	{
		for(unsigned int y = 1; y < volumeSideLength-1; ++y)
		{
			for(unsigned int x = 1; x < volumeSideLength-1; ++x)
			{
				volIter.setPosition(x,y,z);

				//We check the red channel, but in a greyscale image they should all be the same.
				int height = qRed(image.pixel(x,z)); 

				//Set the voxel based on the height.
				if(y < height)
				{
					volIter.setVoxel(arguments.undergroundMaterialID);
				}
				else if(y < height + arguments.surfaceThickness)
				{
					volIter.setVoxel(arguments.surfaceMaterialID);
				}

				//Zero the faces
				//FIXME - looks like a bug - shouldn't be -2?
				//if((x == 0) || (y == 0) || (z == 0) || (x == volumeSideLength-2) || (y == volumeSideLength-2) || (z == volumeSideLength-2))
				//{
				//	volIter.setVoxel(0);
				//}
			}
		}
	}

	ofstream file (arguments.outputFilename.toStdString().c_str(), ios::out|ios::binary);
	saveVolumeRle(file, volData);
	file.close();

	//return app.exec();
	return 0;
}