#pragma region License
/******************************************************************************
This file is part of the Thermite 3D game engine
Copyright (C) 2006  David Williams

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
******************************************************************************/
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
