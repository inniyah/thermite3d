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

#ifndef __MapHandler_H__
#define __MapHandler_H__

#include "DotSceneHandler.h"
#include "ThermiteForwardDeclarations.h"

#include <OgrePrerequisites.h>

#include <QXmlSimpleReader>
#include <QXmlDefaultHandler>

namespace Thermite
{
	class MapHandler : public DotSceneHandler
	{
	public:
		//Rather than just taking a SceneManager, this DotSceneHandler subclass
		//requires a Thermite Map so it can also set up physics on the entities.
		MapHandler(Map* map);

		bool startElement(const QString& namespaceURI, const QString& localName, const QString& qName, const QXmlAttributes &attributes);

	protected:
		virtual Ogre::Entity* handleEntity(const QXmlAttributes &attributes);
		virtual void* handleVolume(const QXmlAttributes &attributes);
		virtual void* handleVoxel(const QXmlAttributes &attributes);

	private:
		Map* mMap;
	};
}

#endif //__MapHandler_H__
