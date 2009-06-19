#ifndef __MapHandler_H__
#define __MapHandler_H__

#include <OgrePrerequisites.h>

#include <QXmlSimpleReader>
#include <QXmlDefaultHandler>

#include "DotSceneHandler.h"
#include "ThermiteForwardDeclarations.h"

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

	private:
		Map* mMap;
	};
}

#endif //__MapHandler_H__
