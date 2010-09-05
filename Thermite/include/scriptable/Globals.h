#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <QColor>
#include <QObject>
#include <QTime>
#include <QScriptEngine>
#include <QVector3D>

namespace Thermite
{
	class Globals : public QObject
	{
		Q_OBJECT

	public:
		Globals(QObject* parent = 0);

		Q_PROPERTY(int timeSinceAppStart READ timeSinceAppStart)

	public slots:
		int timeSinceAppStart(void) const;

	private:
		QTime mTimeSinceAppStart;
	};

	extern Globals globals;
}

#endif //GLOBALS_H_