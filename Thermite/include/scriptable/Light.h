#ifndef LIGHT_H_
#define LIGHT_H_

#include "Object.h"

#include <QColor>
#include <QScriptEngine>
#include <QVector3D>

class Light : public Object
{
	Q_OBJECT

public:
	Light(QObject* parent = 0);

	Q_PROPERTY(QColor colour READ getColour WRITE setColour)

	const QColor& getColour(void) const;
	void setColour(const QColor& col);

private:
	QColor m_colColour;
};

Q_SCRIPT_DECLARE_QMETAOBJECT(Light, QObject*)


#endif //LIGHT_H_