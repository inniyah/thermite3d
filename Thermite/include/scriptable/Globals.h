#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <QColor>
#include <QObject>
#include <QTime>
#include <QScriptEngine>
#include <QVector3D>

#include <cstdint>

class QMutex;

namespace Thermite
{
	class Globals : public QObject
	{
		Q_OBJECT

	public:
		Globals(QObject* parent = 0);
		~Globals();

		Q_PROPERTY(int timeSinceAppStart READ timeSinceAppStart)
		Q_PROPERTY(uint32_t timeStamp READ timeStamp)

	public slots:
		int timeSinceAppStart(void) const;
		uint32_t timeStamp(void);

	private:
		QTime mTimeSinceAppStart;
		uint32_t mTimeStamp;

		QMutex* mTimeStampMutex;
	};

	extern Globals globals;
}

#endif //GLOBALS_H_