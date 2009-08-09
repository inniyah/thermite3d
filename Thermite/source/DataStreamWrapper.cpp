#include "DataStreamWrapper.h"

namespace Thermite
{
	DataStreamWrapper::DataStreamWrapper(const Ogre::DataStreamPtr &dsp)
		:m_Dsp(dsp)
	{
	}

	std::streamsize DataStreamWrapper::showmanyc (void)
	{
			return -1;
	}

	std::streamsize DataStreamWrapper::xsgetn(char* s, std::streamsize n)
	{
			return m_Dsp->read(s,n);
	}	

	std::streamsize DataStreamWrapper::xsputn(const char_type*, std::streamsize)
	{
		throw std::ios::failure("Cannot write to an Ogre::DataStream");
		return -1;
	}

	std::streamsize DataStreamWrapper::_Xsgetn_s(char *s, size_t size, std::streamsize n)
	{
			return m_Dsp->read(s,n);
	}
}