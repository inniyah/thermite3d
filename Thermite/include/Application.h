#ifndef QTOGRE_APPLICATION_H_
#define QTOGRE_APPLICATION_H_

#include <OgreLog.h>

#include <QApplication>

class QSettings;

namespace QtOgre
{
	enum SettingsDialogMode
	{
		DisplaySettingsDialog,
		SuppressSettingsDialog
	};

	enum IgnoredConfigWarningMode
	{
		WarnAboutIgnoredConfigs,
		DoNotWarnAboutIgnoredConfigs
	};

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
	/// 	QtOgre::Application app(argc, argv, new MyGameLogic);
	/// 	return app.exec();
	/// }
	/// \endcode
	/// Where \c MyGameLogic is a QtOgre::GameLogic subclass
	/// \author David Williams
	////////////////////////////////////////////////////////////////////////////////
	class Application : public QApplication, public Ogre::LogListener
	{
		Q_OBJECT

	public:
		/// Creates an instance of the Application class.
		Application(int & argc, char ** argv, GameLogic* gameLogic = 0, IgnoredConfigWarningMode ignoredConfigWarningMode = WarnAboutIgnoredConfigs);
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
		/// Gets the log used by the QtOgre framework.
		Log* _systemLog(void) const;
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
		
		///\name Other
		//@{
		///Creates a new log with a given name.
		Log* createLog(const QString& name);
		///Get the log with the specified name.
		Log* getLogByName(const QString& name);
		///Adds a new page to the settings dialog
		void addSettingsWidget(const QString& title, AbstractSettingsWidget* settingsWidget);
		//@}
		
		//Static functions
		/// Start the main event loop.
		static int exec(SettingsDialogMode eDialogMode = DisplaySettingsDialog);
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

		/// Hides the FPS counter window.
		void hideFPSCounter(void);
		/// Hides the LogManager window.
		void hideLogManager(void);
		/// Hides the settings dialog.
		void hideSettingsDialog(void);

		/// Shows the FPS counter window.
		void showFPSCounter(void);
		/// Shows the LogManager window.
		void showLogManager(void);
		/// Shows the settings dialog.
		int showSettingsDialog(void);

	private:
		//Private functions
		void initialiseLogging(void);
		void initialiseOgre(void);
		///Implemented from Ogre::LogListener to redirect logging 
		void messageLogged(const Ogre::String& message, Ogre::LogMessageLevel lml, bool maskDebug, const Ogre::String& logName);
		void loadResourcePathsFromConfigFile(const QString& filename);

		//Widgets
		FPSDialog* mFPSDialog;
		GraphicsSettingsWidget* mGraphicsSettingsWidget;
		EventHandlingOgreWidget* mOgreWidget;
		SettingsDialog* mSettingsDialog;

		//Logging
		Ogre::Log* mInternalOgreLog;
		Ogre::LogManager* mInternalOgreLogManager;
		LogManager* mLogManager;
		Log* mOgreLog;
		Log* mSystemLog;

		//Ogre Stuff
		Ogre::RenderSystem* mActiveRenderSystem;
		Ogre::RenderSystem* mOpenGLRenderSystem;
		Ogre::RenderSystem* mDirect3D9RenderSystem;
		Ogre::Root* mRoot;

		//Config warnings
		void warnAboutIgnoredConfigFile(const QString& filename);

		//Misc
		unsigned int mFrameCounter;
		GameLogic* mGameLogic;
		QTimer* mAutoUpdateTimer;
		QSettings* mSettings;
		bool mAutoUpdateEnabled;
		bool mIsInitialised;
		IgnoredConfigWarningMode mIgnoredConfigWarningMode;
	};
}

//This redefines qApp, causing it to return an Application pointer instead of a QApplication one.
//This is useful, for example, to access the logging system. This is done in the same way that
//Qt does it to get a QApplication rather than a QCoreApplication, so it should be legitimate.
#if defined(qApp)
#undef qApp
#endif
#define qApp (static_cast<QtOgre::Application *>(QCoreApplication::instance()))

#endif /*QTOGRE_APPLICATION_H_*/
