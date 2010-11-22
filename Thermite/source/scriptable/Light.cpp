#include "Light.h"

namespace Thermite
{
	Light::Light(QObject* parent)
		:Object(parent)
		,m_colColour(255,255,255)
		,mType(PointLight)
	{
	}

	const QColor& Light::getColour(void) const
	{
		return m_colColour;
	}

	void Light::setColour(const QColor& col)
	{
		m_colColour = col;
		setModified(true);
	}

	Light::LightType Light::getType(void) const
	{
		return mType;
	}

	void Light::setType(Light::LightType type)
	{
		mType = type;
		setModified(true);
	}
}