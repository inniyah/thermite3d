#include "RenderComponent.h"

#include "OgreRoot.h"

namespace Thermite
{
	RenderComponent::RenderComponent(Object* parent)
		:Component(parent)
		,mOgreSceneNode(0)
	{
	}

	RenderComponent::~RenderComponent(void)
	{
	}

	void RenderComponent::update(void)
	{
		std::string objAddressAsString = QString::number(reinterpret_cast<qulonglong>(mParent), 16).toStdString();
		if(mOgreSceneNode == 0)
		{
			std::string sceneNodeName(objAddressAsString + "_SceneNode");
			Ogre::SceneManager* sceneManager = Ogre::Root::getSingletonPtr()->getSceneManager("OgreSceneManager");
			mOgreSceneNode = sceneManager->getRootSceneNode()->createChildSceneNode(sceneNodeName);
		}

		if(mOgreSceneNode)
		{
			QMatrix4x4 qtTransform = mParent->transform();
			Ogre::Matrix4 ogreTransform;
			for(int row = 0; row < 4; ++row)
			{
				Ogre::Real* rowPtr = ogreTransform[row];
				for(int col = 0; col < 4; ++col)
				{
					Ogre::Real* colPtr = rowPtr + col;
					*colPtr = qtTransform(row, col);
				}
			}

			mOgreSceneNode->setOrientation(ogreTransform.extractQuaternion());
			mOgreSceneNode->setPosition(ogreTransform.getTrans());

			QVector3D scale = mParent->size();
			mOgreSceneNode->setScale(Ogre::Vector3(scale.x(), scale.y(), scale.z()));

			mOgreSceneNode->setVisible(true);
		}
	}
}
