#include "DotSceneWithVolumeHandler.h"

#include <OgreEntity.h>
#include <OgreSceneManager.h>

DotSceneWithVolumeHandler::DotSceneWithVolumeHandler(Ogre::SceneManager* sceneManager)
:DotSceneHandler(sceneManager)
{
	//mSceneManager = sceneManager;
}

/*bool DotSceneWithVolumeHandler::startElement(const QString & ,
								   const QString & ,
								   const QString &qName,
								   const QXmlAttributes &attributes)
{
	qDebug(qName.toStdString().c_str());
	
	if(qName == "entity")
	{
		Ogre::Entity* entity = mSceneManager->createEntity(attributes.value("name").toStdString(), attributes.value("meshFile").toStdString());
		mCurrentNode->attachObject(entity);
	}
	if(qName == "node")
	{
		mCurrentNode = mCurrentNode->createChildSceneNode(attributes.value("name").toStdString());
	}
	if(qName == "nodes")
	{
		mCurrentNode = mSceneManager->getRootSceneNode();
	}
	if(qName == "position")
	{
		mCurrentNode->setPosition(attributes.value("x").toFloat(), attributes.value("y").toFloat(), attributes.value("z").toFloat());
	}

	return true;
}*/