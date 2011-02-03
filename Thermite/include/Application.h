#ifndef THERMITE_APPLICATION_H_
#define THERMITE_APPLICATION_H_

#include <OgreLog.h>

#include <QApplication>

class QSettings;

namespace Thermite
{
	class EventHandlingOgreWidget;
	class FPSDialog;
	class GameLogic;
	class GraphicsSettingsWidget;
	class Log;
	class LogManager;
	class SettingsDialog;
	class AbstractSettingsWidget;
	
	////////////////////////////////////////////////////////////////////////////////
	/// The entry point for QtOgre
	/// 
	/// Usage:
	/// \code
	/// int main(int argc, char *argv[])
	/// {
	/// 	Application app(argc, argv, new MyGameLogic);
	/// 	return app.exec();
	/// }
	/// \endcode
	/// Where \c MyGameLogic is a GameLogic subclass
	/// \author David Williams
	////////////////////////////////////////////////////////////////////////////////
	class Application : public QApplication
	{
		Q_OBJECT

	public:
		/// Creates an instance of the Application class.
		Application(int & argc, char ** argv, GameLogic* gameLogic = 0);
		/// Destroys an instance of the Application class
		~Application();

		///\name Getters
		//@{
		/// The total number of frames rendered
		unsigned int frameCount(void) const;
		/// Get the OGRE RenderWindow for adding viewports   
		Ogre::RenderWindow* ogreRenderWindow(void) const;
		/// Get the main window widget
		QWidget* mainWidget(void) const;
		/// Access the application settings
		QSettings* settings(void) const;
		//@}

		///\name Setters
		//@{
		/// Sets the period between sucessive updates.
		void setAutoUpdateInterval(int intervalInMilliseconds);
		/// Controls whether QtOgre periodically calls update().
		void setAutoUpdateEnabled(bool autoUpdateEnabled);
		//@}

		///\name Testers
		//@{
		/// Determine whether the OpenGL render system is available
		bool isOpenGLAvailable(void) const;
		/// Determine whether the Direct3D9 render system is available.
		bool isDirect3D9Available(void) const;
		//@}
		
		//Static functions
		/// Start the main event loop.
		static int exec();
		/// Utility function to center a widget.
		static void centerWidget(QWidget* widgetToCenter, QWidget* parent = 0);

		/// Shows a message box with an 'Info' icon and 'Information' in the title bar.
		static void showInfoMessageBox(const QString& text);
		/// Shows a message box with a 'Warning' icon and 'Warning' in the title bar.
		static void showWarningMessageBox(const QString& text);
		/// Shows a message box with an 'Error' icon and 'Error' in the title bar.
		static void showErrorMessageBox(const QString& text);

	public slots:
		void applySettings(void);
		void initialise(void);
		void shutdown(void);
		void update(void);	

	private:
		void initialiseOgre(void);

		//Widgets
		EventHandlingOgreWidget* mOgreWidget;

		//Ogre Stuff
		Ogre::RenderSystem* mActiveRenderSystem;
		Ogre::RenderSystem* mOpenGLRenderSystem;
		Ogre::RenderSystem* mDirect3D9RenderSystem;
		Ogre::Root* mRoot;

		//Misc
		unsigned int mFrameCounter;
		GameLogic* mGameLogic;
		QTimer* mAutoUpdateTimer;
		QSettings* mSettings;
		bool mAutoUpdateEnabled;
		bool mIsInitialised;
	};
}

//This redefines qApp, causing it to return an Application pointer instead of a QApplication one.
//This is useful, for example, to access the logging system. This is done in the same way that
//Qt does it to get a QApplication rather than a QCoreApplication, so it should be legitimate.
#if defined(qApp)
#undef qApp
#endif
#define qApp (static_cast<Thermite::Application *>(QCoreApplication::instance()))

#endif /*THERMITE_APPLICATION_H_*/
