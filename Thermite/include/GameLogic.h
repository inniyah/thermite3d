#ifndef QTOGRE_GAMELOGIC_H_
#define QTOGRE_GAMELOGIC_H_

#include "EventHandler.h"

namespace QtOgre
{
	class Application;
	
	/**
	 * The game controller
	 * 
	 * You'll need to subclass this to enable control over the flow of your game.
	 * 
	 * Most likely, you'll want to override each of the functions in this class to provide the logic for the application. 
	 * 
	 * \author David Williams
	 */
	class GameLogic : public EventHandler
	{
		friend class Application; //So the application can set itself.
	public:
		/**
		 * Creates an instance of the GameLogic class.
		 */
		GameLogic(void);
		
	public:
		/**
		 * Initialise the game logic
		 */
		virtual void initialise(void);
		/**
		 * Update the game
		 */
		virtual void update(void);
		/**
		 * Shut down the game
		 */
		virtual void shutdown(void);

		virtual void onKeyPress(QKeyEvent* event);
		virtual void onKeyRelease(QKeyEvent* event);

		virtual void onMousePress(QMouseEvent* event);
		virtual void onMouseRelease(QMouseEvent* event);
		virtual void onMouseDoubleClick(QMouseEvent* event);
		virtual void onMouseMove(QMouseEvent* event);

		virtual void onWheel(QWheelEvent* event);

	protected:
		Application* mApplication;
	};
}

#endif /*QTOGRE_GAMELOGIC_H_*/
