#include "DotSceneWithVolumeHandler.h"

#include <OgreEntity.h>
#include <OgreSceneManager.h>

#include "PhysicalEntity.h"
#include "World.h"

DotSceneWithVolumeHandler::DotSceneWithVolumeHandler(World* world)
:DotSceneHandler(world->m_pOgreSceneManager)
,mWorld(world)
{
	//mSceneManager = sceneManager;
}

Ogre::Entity* DotSceneWithVolumeHandler::handleEntity(const QXmlAttributes &attributes)
{
	Ogre::Entity* entity = DotSceneHandler::handleEntity(attributes);

	PhysicalEntity* physObj = new PhysicalEntity(mWorld, entity);

	return entity;
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