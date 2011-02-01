#include "EventHandlingOgreWidget.h"

#include "Application.h"
#include "EventHandler.h"
#include "Log.h"

#include <OgreRenderWindow.h>
#include <OgreRoot.h>
#include <OgreStringConverter.h>

#include <QCloseEvent>

#if defined(Q_WS_X11)
#include <QX11Info>
#endif

namespace QtOgre
{
	EventHandlingOgreWidget::EventHandlingOgreWidget(QWidget* parent, Qt::WindowFlags f)
	:OgreWidget(parent, f)
	,mEventHandler(0)
	{		
	}

	EventHandlingOgreWidget::~EventHandlingOgreWidget()
	{
	}

	void EventHandlingOgreWidget::setEventHandler(EventHandler* eventHandler)
	{
		mEventHandler = eventHandler;
	}

	void EventHandlingOgreWidget::closeEvent(QCloseEvent *event)
	{
		//We ignore this event because we wish to keep the MainWindow
		//open so that the log file can be seen duing shutdown.
		event->ignore();
		qApp->shutdown();		
	}

	void EventHandlingOgreWidget::keyPressEvent(QKeyEvent* event)
	{
		if(mEventHandler != 0)
		{
			mEventHandler->onKeyPress(event);
		}
		else
		{
			QWidget::keyPressEvent(event);
		}
	}

	void EventHandlingOgreWidget::keyReleaseEvent(QKeyEvent* event)
	{
		if(mEventHandler != 0)
		{
			mEventHandler->onKeyRelease(event);
		}
		else
		{
			QWidget::keyReleaseEvent(event);
		}
	}

	void EventHandlingOgreWidget::mousePressEvent(QMouseEvent* event)
	{
		if(mEventHandler != 0)
		{
			mEventHandler->onMousePress(event);
		}
		else
		{
			QWidget::mousePressEvent(event);
		}
	}

	void EventHandlingOgreWidget::mouseReleaseEvent(QMouseEvent* event)
	{
		if(mEventHandler != 0)
		{
			mEventHandler->onMouseRelease(event);
		}
		else
		{
			QWidget::mouseReleaseEvent(event);
		}
	}

	void EventHandlingOgreWidget::mouseDoubleClickEvent(QMouseEvent* event)
	{
		if(mEventHandler != 0)
		{
			mEventHandler->onMouseDoubleClick(event);
		}
		else
		{
			QWidget::mouseDoubleClickEvent(event);
		}
	}

	void EventHandlingOgreWidget::mouseMoveEvent(QMouseEvent* event)
	{
		if(mEventHandler != 0)
		{
			mEventHandler->onMouseMove(event);
		}
		else
		{
			QWidget::mouseMoveEvent(event);
		}
	}

	void EventHandlingOgreWidget::wheelEvent(QWheelEvent* event)
	{
		if(mEventHandler != 0)
		{
			mEventHandler->onWheel(event);
		}
		else
		{
			QWidget::wheelEvent(event);
		}
	}
}
