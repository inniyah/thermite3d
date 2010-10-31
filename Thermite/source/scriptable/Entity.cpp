#include "Entity.h"

namespace Thermite
{
	Entity::Entity(QObject* parent)
		:Object(parent)
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
