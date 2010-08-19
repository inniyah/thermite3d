#pragma region License
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
#pragma endregion

#include "DataStreamWrapper.h"
#include "MapHandler.h"
#include "MapManager.h"
#include "MapResource.h"

using namespace PolyVox;

namespace Thermite
{
	MapResource::MapResource(	Ogre::ResourceManager* creator, const Ogre::String &name, 
								Ogre::ResourceHandle handle, const Ogre::String &group, bool isManual, 
								Ogre::ManualResourceLoader *loader)
		:Ogre::Resource (creator, name, handle, group, isManual, loader)
		,m_pMap(0)
	{		
		createParamDictionary ("Map");
	}

	MapResource::~MapResource()
	{
		unload ();
	}	

	void MapResource::loadImpl ()
	{
		m_pMap = new Map;

		Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton ().openResource (mName, mGroup, true, this);
		std::string contents = stream->getAsString();

		MapHandler handler(m_pMap);
		QXmlSimpleReader reader;
		reader.setContentHandler(&handler);
		reader.setErrorHandler(&handler);

		QXmlInputSource xmlInputSource;
		xmlInputSource.setData(QString::fromStdString(contents));
		reader.parse(xmlInputSource);
	}

	void MapResource::unloadImpl ()
	{
		//Clear the pointer
		delete m_pMap;
		m_pMap = 0;
	}

	size_t MapResource::calculateSize () const
	{
		//NOTE - I don't really know what this function is for,
		//so am therefore a bit vague on how to implement it.
		return 42; //The answer...
	}
}
