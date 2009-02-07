#include "DotSceneWithVolumeHandler.h"

#include <OgreEntity.h>
#include <OgreSceneManager.h>

#include "PhysicalObject.h"
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

	//The entities parent SceneNode will be manipulated by the physics engine. However, it is
	//possible this parent will have many children, and we don't want all of them to be affected.
	//Therefore we create an intermediary SceneNode which will have only our Entity as a child.
	//Ogre::SceneNode* parentSceneNode = entity->getParentSceneNode();
	//parentSceneNode->detachObject(entity);
	//Ogre::SceneNode* intermediarySceneNode = parentSceneNode->createChildSceneNode();
	//intermediarySceneNode->attachObject(entity);

	PhysicalObject* physObj = new PhysicalObject(mWorld, entity);

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