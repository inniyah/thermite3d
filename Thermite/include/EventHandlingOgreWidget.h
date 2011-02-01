#ifndef QTOGRE_EVENTHANDLINGOGREWIDGET_H_
#define QTOGRE_EVENTHANDLINGOGREWIDGET_H_

#include "OgreWidget.h"

class QWidget;

namespace QtOgre
{
	class EventHandler;

	class EventHandlingOgreWidget : public OgreWidget
	{
		Q_OBJECT

	public:
		EventHandlingOgreWidget(QWidget* parent=0, Qt::WindowFlags f=0);
		~EventHandlingOgreWidget();

		void setEventHandler(EventHandler* eventHandler);

		void closeEvent(QCloseEvent *event);

		void keyPressEvent(QKeyEvent* event);
		void keyReleaseEvent(QKeyEvent* event);

		void mousePressEvent(QMouseEvent* event);
		void mouseReleaseEvent(QMouseEvent* event);
		void mouseDoubleClickEvent(QMouseEvent* event);
		void mouseMoveEvent(QMouseEvent* event);

		void wheelEvent(QWheelEvent* event);

	private:
		EventHandler* mEventHandler;
	};
}

#endif /*QTOGRE_EVENTHANDLINGOGREWIDGET_H_*/
