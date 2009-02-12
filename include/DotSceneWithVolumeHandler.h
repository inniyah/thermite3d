#ifndef __DotSceneWithVolumeHandler_H__
#define __DotSceneWithVolumeHandler_H__

#include <OgrePrerequisites.h>

#include <QXmlSimpleReader>
#include <QXmlDefaultHandler>

#include "DotSceneHandler.h"
#include "ThermiteForwardDeclarations.h"

class DotSceneWithVolumeHandler : public DotSceneHandler
{
public:
	//Rather than just taking a SceneManager, this DotSceneHandler subclass
	//requires a Thermite World so it can also set up physics on the entities.
	DotSceneWithVolumeHandler(World* world);

protected:
	virtual Ogre::Entity* handleEntity(const QXmlAttributes &attributes);

private:
	World* mWorld;
};

#endif //__DotSceneWithVolumeHandler_H__
