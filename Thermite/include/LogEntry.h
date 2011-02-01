#ifndef QTOGRE_LOGENTRY_H_
#define QTOGRE_LOGENTRY_H_

#include "LogLevel.h"

#include <QString>
#include <QTime>
#include <QVariant>

namespace QtOgre
{
	class LogEntry
	{
	public:
		LogEntry(const QString &msg, LogLevel level);

		LogLevel getLevel(void);
		const QString& getMessage(void);
		const QTime& getTimestamp(void);

	private:
		QString mMsg;
		QTime mTimestamp;
		LogLevel mLevel;		
	};
}
#endif