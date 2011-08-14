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
	
	Entity::~Entity()
	{
		if(mOgreEntity)
		{
			mOgreSceneNode->detachObject(mOgreEntity);
			mSceneManager->destroyMovableObject(mOgreEntity);
		}
	}

	void Entity::update(void)
	{
		RenderComponent::update();							

		//Set a custom material if necessary
		if(materialName().isEmpty() == false)
		{
			if(mOgreEntity)
			{
				//NOTE: Might be sensible to check if this really need setting, perhaps it is slow.
				//But you can only get materials from SubEntities.
				mOgreEntity->setMaterialName(materialName().toAscii().constData());
			}
		}

		//Animation
		if(mOgreEntity)
		{
			Ogre::AnimationStateSet* animationStateSet = mOgreEntity->getAllAnimationStates();		
			if(animationStateSet && animationStateSet->hasAnimationState(animationName().toAscii().constData()))
			{
				Ogre::AnimationState* animationState = animationStateSet->getAnimationState(animationName().toAscii().constData());
				animationState->setEnabled(animated());
				animationState->setLoop(loopAnimation());
			}
		}
	}

	const QString& Entity::meshName(void) const
	{
		return mMeshName;
	}

	void Entity::setMeshName(const QString& name)
	{
		mMeshName = name;

		//Should delete old mesh first!!!

		if(mMeshName.isEmpty() == false)
		{
			std::string objAddressAsString = QString::number(reinterpret_cast<qulonglong>(mParent), 16).toAscii();
			std::string entityName(objAddressAsString + "_Entity");
			mOgreEntity = mSceneManager->createEntity(entityName, meshName().toAscii().constData());
			mOgreSceneNode->attachObject(mOgreEntity);
		}
	}

	const QString& Entity::materialName(void) const
	{
		return mMaterialName;
	}

	void Entity::setMaterialName(const QString& name)
	{
		mMaterialName = name;
	}

	const bool Entity::animated(void) const
	{
		return mAnimated;
	}

	void Entity::setAnimated(bool animated)
	{
		mAnimated = animated;
	}

	const QString& Entity::animationName(void) const
	{
		return mAnimationName;
	}

	void Entity::setAnimationName(const QString& name)
	{
		mAnimationName = name;
	}

	const bool Entity::loopAnimation(void) const
	{
		return mLoopAnimation;
	}

	void Entity::setLoopAnimation(bool loopAnimation)
	{
		mLoopAnimation = loopAnimation;
	}
}
