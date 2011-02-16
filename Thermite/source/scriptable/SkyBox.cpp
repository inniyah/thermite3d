#include "SkyBox.h"

namespace Thermite
{
	SkyBox::SkyBox(Object* parent)
		:RenderComponent(parent)
	{
		mMaterialName = "";
	}

	const QString& SkyBox::materialName(void) const
	{
		return mMaterialName;
	}

	void SkyBox::setMaterialName(const QString& name)
	{
		mMaterialName = name;
		mParent->setModified(true);
	}
}
