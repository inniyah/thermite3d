#ifndef __DotSceneWithVolumeHandler_H__
#define __DotSceneWithVolumeHandler_H__

#include <OgrePrerequisites.h>

#include <QXmlSimpleReader>
#include <QXmlDefaultHandler>

#include "DotSceneHandler.h"

class DotSceneWithVolumeHandler : public DotSceneHandler
{
public:
	DotSceneWithVolumeHandler(Ogre::SceneManager* sceneManager);

	//bool startElement(const QString &, const QString &, const QString &qName, const QXmlAttributes &attributes);

private:
	//Ogre::SceneManager* mSceneManager;
	//Ogre::SceneNode* mCurrentNode;
};

#endif //__DotSceneWithVolumeHandler_H__
