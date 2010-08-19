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
#include "ScriptManager.h"
#include "ScriptResource.h"

#include "MaterialDensityPair.h"

#include "OgreVector3.h"
#include "OgreLogManager.h"
#include "OgreStringConverter.h"

#include <iostream> //FIXME - remove this...

using namespace PolyVox;
using namespace std;

namespace Thermite
{
	ScriptResource::ScriptResource(	Ogre::ResourceManager* creator, const Ogre::String &name, 
									Ogre::ResourceHandle handle, const Ogre::String &group, bool isManual, 
									Ogre::ManualResourceLoader *loader)
		:Ogre::Resource (creator, name, handle, group, isManual, loader)
	{		
		createParamDictionary ("Script");
	}

	ScriptResource::~ScriptResource()
	{
		unload();
	}

	std::string ScriptResource::getScriptData(void) const
	{
		return mScriptData;
	}

	void ScriptResource::loadImpl ()
	{
		Ogre::DataStreamPtr stream = Ogre::ResourceGroupManager::getSingleton ().openResource (mName, mGroup, true, this);
		mScriptData = stream->getAsString();
	}

	void ScriptResource::unloadImpl ()
	{
		mScriptData = "";
	}
	
	size_t ScriptResource::calculateSize () const
	{
		//NOTE - I don't really know what this function is for,
		//so am therefore a bit vague on how to implement it.
		return 42; //The answer...
	}
}
