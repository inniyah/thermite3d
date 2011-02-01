#ifndef QTOGRE_EVENTHANDLER_H_
#define QTOGRE_EVENTHANDLER_H_

class QKeyEvent;
class QMouseEvent;
class QWheelEvent;

namespace QtOgre
{
	/**
	 * Base for event handlers
	 * 
	 * This is an abstract base class.
	 * 
	 * \author David Williams
	 */
	class EventHandler
	{
	public:	
		virtual void onKeyPress(QKeyEvent* event) = 0;
		virtual void onKeyRelease(QKeyEvent* event) = 0;

		virtual void onMousePress(QMouseEvent* event) = 0;
		virtual void onMouseRelease(QMouseEvent* event) = 0;
		virtual void onMouseDoubleClick(QMouseEvent* event) = 0;
		virtual void onMouseMove(QMouseEvent* event) = 0;

		virtual void onWheel(QWheelEvent* event) = 0;
	};
}

#endif /*QTOGRE_EVENTHANDLER_H_*/
