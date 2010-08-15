#ifndef GLOBALS_H_
#define GLOBALS_H_

#include <QColor>
#include <QObject>
#include <QScriptEngine>
#include <QVector3D>

class Globals : public QObject
{
	Q_OBJECT

public:
	Globals(QObject* parent = 0);

	Q_PROPERTY(quint32 currentFrameTime READ getCurrentFrameTime)
	Q_PROPERTY(quint32 previousFrameTime READ getPreviousFrameTime)

public slots:
	quint32 getCurrentFrameTime(void) const;
	void setCurrentFrameTime(const quint32 uCurrentFrameTime);

	quint32 getPreviousFrameTime(void) const;
	void setPreviousFrameTime(const quint32 uPreviousFrameTime);
private:
	quint32 m_uCurrentFrameTime;
	quint32 m_uPreviousFrameTime;
};

#endif //GLOBALS_H_