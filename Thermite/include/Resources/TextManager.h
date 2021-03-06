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

#ifndef __TextManager_H__
#define __TextManager_H__

#include <OgreResourceManager.h>
#include "TextResource.h"

#include "VolumeSerializationProgressListenerImpl.h"

namespace Thermite
{
	class TextManager : public Ogre::ResourceManager, public Ogre::Singleton<TextManager>
	{
	protected:
		// must implement this from ResourceManager's interface
		Ogre::Resource *createImpl(const Ogre::String &name, Ogre::ResourceHandle handle, 
			const Ogre::String &group, bool isManual, Ogre::ManualResourceLoader *loader, 
			const Ogre::NameValuePairList *createParams);

		TextResourcePtr create (const Ogre::String& name, const Ogre::String& group,
			bool isManual = false, Ogre::ManualResourceLoader* loader = 0,
			const Ogre::NameValuePairList* createParams = 0);

		TextResourcePtr getByName(const Ogre::String& name, const Ogre::String& groupName = Ogre::RGN_DEFAULT);

	public:
		TextManager ();
		virtual ~TextManager ();

		virtual TextResourcePtr load (const Ogre::String &name, const Ogre::String &group);

		static TextManager &getSingleton ();
		static TextManager *getSingletonPtr ();
	};
}

#endif //__TextManager_H__
