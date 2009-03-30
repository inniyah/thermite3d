#include "Utility.h"

#include "PolyVoxCore/PolyVoxImpl/CPlusPlusZeroXSupport.h"

#include <iomanip>
#include <sstream>

namespace Thermite
{
	std::string generateUID(const std::string& prefix)
	{
		//This will be incremented each time
		static PolyVox::uint32_t currentID = 0;

		//We'll split it just to make it more readable
		PolyVox::uint16_t lowerBits = currentID & 0x0000FFFF;
		PolyVox::uint16_t upperBits = currentID >> 16;

		std::stringstream ss;
		//hex and uppercase just get set once. Fill and width seem to need to be set twice.
		ss << prefix 
			<< "-" 
			<< std::hex << std::uppercase
			<< std::setw(4) << std::setfill('0') << upperBits 
			<< "-" 
			<< std::setw(4) << std::setfill('0') << lowerBits;

		//Inrement the counter and return the string.
		++currentID;
		return ss.str();
	}
}