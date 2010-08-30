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

#ifndef __ScriptResource_H__
#define __ScriptResource_H__

#include "OgrePrerequisites.h"
#include "OgreSharedPtr.h"

#include "PolyVoxForwardDeclarations.h"
#include "PolyVoxImpl/TypeDef.h"
#include "Volume.h"

#include <OgreResourceManager.h>

namespace Thermite
{
	class ScriptResource : public Ogre::Resource
	{
	public:		
		ScriptResource (Ogre::ResourceManager *creator, const Ogre::String &name, 
			Ogre::ResourceHandle handle, const Ogre::String &group, bool isManual = false, 
			Ogre::ManualResourceLoader *loader = 0);
		~ScriptResource();		

		std::string getScriptData(void) const;

	protected:

		// must implement these from the Ogre::Resource interface
		void loadImpl ();
		void unloadImpl ();
		size_t calculateSize () const;

	private:
		std::string mScriptData;
	};

	class ScriptResourcePtr : public Ogre::SharedPtr<ScriptResource> 
	{
	public:
		ScriptResourcePtr () : Ogre::SharedPtr<ScriptResource> () {}
		explicit ScriptResourcePtr (ScriptResource *rep) : Ogre::SharedPtr<ScriptResource> (rep) {}
		ScriptResourcePtr (const ScriptResourcePtr &r) : Ogre::SharedPtr<ScriptResource> (r) {} 
		ScriptResourcePtr (const Ogre::ResourcePtr &r) : Ogre::SharedPtr<ScriptResource> ()
		{
			if(r.isNull())
				return;

			// lock & copy other mutex pointer
			OGRE_LOCK_MUTEX (*r.OGRE_AUTO_MUTEX_NAME)
				OGRE_COPY_AUTO_SHARED_MUTEX (r.OGRE_AUTO_MUTEX_NAME)
				pRep = static_cast<ScriptResource*> (r.getPointer ());
			pUseCount = r.useCountPointer ();
			useFreeMethod = r.freeMethod();
			if (pUseCount)
			{
				++ (*pUseCount);
			}
		}

		/// Operator used to convert a ResourcePtr to a ScriptResourcePtr
		ScriptResourcePtr& operator=(const Ogre::ResourcePtr& r)
		{
			if (pRep == static_cast<ScriptResource*> (r.getPointer ()))
				return *this;
			release ();

			if(r.isNull())
				return *this;

			// lock & copy other mutex pointer
			OGRE_LOCK_MUTEX (*r.OGRE_AUTO_MUTEX_NAME)
				OGRE_COPY_AUTO_SHARED_MUTEX(r.OGRE_AUTO_MUTEX_NAME)
				pRep = static_cast<ScriptResource*> (r.getPointer());
			pUseCount = r.useCountPointer ();
			useFreeMethod = r.freeMethod();
			if (pUseCount)
			{
				++ (*pUseCount);
			}
			return *this;
		}
	};
}

#endif //__ScriptResource_H__
