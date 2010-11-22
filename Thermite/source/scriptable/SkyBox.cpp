#include "SkyBox.h"

namespace Thermite
{
	SkyBox::SkyBox(QObject* parent)
		:Object(parent)
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
		setModified(true);
	}
}
