#include "Entity.h"

#include "OgreEntity.h"
#include "OgreRoot.h"

namespace Thermite
{
	Entity::Entity(Object* parent)
		:RenderComponent(parent)
		,mOgreEntity(0)
	{
		mAnimated = false;
		mLoopAnimation = false;
		mAnimationName = "";
	}

	void Entity::update(void)
	{
		RenderComponent::update();

		std::string objAddressAsString = QString::number(reinterpret_cast<qulonglong>(mParent), 16).toStdString();

		//Ogre::Entity* ogreEntity;
		std::string entityName(objAddressAsString + "_Entity");

		/*if(mOgreSceneManager->hasEntity(entityName))
		{
			ogreEntity = mOgreSceneManager->getEntity(entityName);
		}
		else*/
		if(!mOgreEntity)
		{
			Ogre::SceneManager* sceneManager = Ogre::Root::getSingletonPtr()->getSceneManager("OgreSceneManager");
			mOgreEntity = sceneManager->createEntity(entityName, meshName().toStdString());
			mOgreSceneNode->attachObject(mOgreEntity);
		}						

		//Set a custom material if necessary
		if(materialName().isEmpty() == false)
		{
			//NOTE: Might be sensible to check if this really need setting, perhaps it is slow.
			//But you can only get materials from SubEntities.
			mOgreEntity->setMaterialName(materialName().toStdString());
		}

		//Animation
		Ogre::AnimationStateSet* animationStateSet = mOgreEntity->getAllAnimationStates();		
		if(animationStateSet && animationStateSet->hasAnimationState(animationName().toStdString()))
		{
			Ogre::AnimationState* animationState = animationStateSet->getAnimationState(animationName().toStdString());
			animationState->setEnabled(animated());
			animationState->setLoop(loopAnimation());
		}
	}

	const QString& Entity::meshName(void) const
	{
		return mMeshName;
	}

	void Entity::setMeshName(const QString& name)
	{
		mMeshName = name;
		mParent->setModified(true);
	}

	const QString& Entity::materialName(void) const
	{
		return mMaterialName;
	}

	void Entity::setMaterialName(const QString& name)
	{
		mMaterialName = name;
		mParent->setModified(true);
	}

	const bool Entity::animated(void) const
	{
		return mAnimated;
	}

	void Entity::setAnimated(bool animated)
	{
		mAnimated = animated;
		mParent->setModified(true);
	}

	const QString& Entity::animationName(void) const
	{
		return mAnimationName;
	}

	void Entity::setAnimationName(const QString& name)
	{
		mAnimationName = name;
		mParent->setModified(true);
	}

	const bool Entity::loopAnimation(void) const
	{
		return mLoopAnimation;
	}

	void Entity::setLoopAnimation(bool loopAnimation)
	{
		mLoopAnimation = loopAnimation;
		mParent->setModified(true);
	}
}
