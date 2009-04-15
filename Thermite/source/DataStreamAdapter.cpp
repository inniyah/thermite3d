#include "DataStreamAdapter.h"

DataStreamAdapter::DataStreamAdapter(const Ogre::DataStreamPtr &dsp)
	:m_Dsp(dsp)
{
}

std::streamsize DataStreamAdapter::showmanyc (void)
{
        return -1;
}

std::streamsize DataStreamAdapter::xsgetn(char* s, std::streamsize n)
{
		return m_Dsp->read(s,n);
}	

std::streamsize DataStreamAdapter::xsputn(const char_type*, std::streamsize)
{
    throw std::ios::failure("Cannot write to an Ogre::DataStream");
	return -1;
}

std::streamsize DataStreamAdapter::_Xsgetn_s(char *s, size_t size, std::streamsize n)
{
		return m_Dsp->read(s,n);
}