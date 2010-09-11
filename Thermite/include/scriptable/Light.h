#ifndef LIGHT_H_
#define LIGHT_H_

#include "Object.h"

#include <QColor>
#include <QScriptEngine>
#include <QVector3D>

namespace Thermite
{
	class Light : public Object
	{
		Q_OBJECT

		Q_ENUMS(LightType)

		Q_PROPERTY(QColor colour READ getColour WRITE setColour)
		Q_PROPERTY(LightType type READ getType WRITE setType)		

	public:		

		enum LightType
		{
			PointLight = 0,
			DirectionalLight = 1,
			SpotLight = 2
		};

		Light(QObject* parent = 0);

		const QColor& getColour(void) const;
		void setColour(const QColor& col);

		LightType getType(void) const;
		void setType(LightType type);

	private:
		QColor m_colColour;
		LightType mType;
	};
}

Q_SCRIPT_DECLARE_QMETAOBJECT(Thermite::Light, QObject*)

#endif //LIGHT_H_