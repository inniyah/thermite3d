#include "Light.h"

Light::Light(QObject* parent)
	:Object(parent)
{
}

const QColor& Light::getColour(void) const
{
	return m_colColour;
}

void Light::setColour(const QColor& col)
{
	m_colColour = col;
}