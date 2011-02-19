#include "Component.h"

namespace Thermite
{
	Component::Component(Object* parent)
		:mParent(parent)
	{
		parent->setComponent(this);
	}

	Component::~Component(void)
	{
	}

	void Component::update(void)
	{
	}
}