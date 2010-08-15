#include "Entity.h"

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
