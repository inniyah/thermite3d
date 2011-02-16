#include "Entity.h"

namespace Thermite
{
	Entity::Entity(Object* parent)
		:RenderComponent(parent)
	{
		mAnimated = false;
		mLoopAnimation = false;
		mAnimationName = "";
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
