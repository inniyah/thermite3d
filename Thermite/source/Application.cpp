#include "Application.h"

#include "EventHandlingOgreWidget.h"
#include "FPSDialog.h"
#include "GraphicsSettingsWidget.h"
#include "LogManager.h"
#include "SettingsDialog.h"

#include <OgreConfigFile.h>
#include <OgreRenderSystem.h>
#include <OgreRoot.h>

#include <QCloseEvent>
#include <QDesktopWidget>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QTimer>
#include <QSettings>

//Q_INIT_RESOURCE cannot be called from within a namespace, so we provide
//this function. See the Q_INIT_RESOURCE documentation for an explanation.
//inline void initQtResources() { Q_INIT_RESOURCE(resources); }

namespace Thermite
{
	void qtMessageHandler(QtMsgType type, const char* msg)
     {
         switch (type)
		 {
         case QtDebugMsg:
             qApp->_systemLog()->logMessage(msg, LL_DEBUG);
             break;
         case QtWarningMsg:
             qApp->_systemLog()->logMessage(msg, LL_WARNING);
             break;
         case QtCriticalMsg:
             qApp->_systemLog()->logMessage(msg, LL_ERROR);
             break;
         case QtFatalMsg:
			 //We don't override this one as we are dying and wouldn't see it.
             fprintf(stderr, "Fatal: %s\n", msg);
             abort();
         }
     }

	Application::Application(int& argc, char** argv, GameLogic* gameLogic, IgnoredConfigWarningMode ignoredConfigWarningMode)
	:QApplication(argc, argv)
	,mFPSDialog(0)
	,mGraphicsSettingsWidget(0)
	,mOgreWidget(0)
	,mSettingsDialog(0)
	,mInternalOgreLog(0)
	,mInternalOgreLogManager(0)
	,mLogManager(0)
	,mOgreLog(0)
	,mSystemLog(0)
	,mActiveRenderSystem(0)
	,mOpenGLRenderSystem(0)
	,mDirect3D9RenderSystem(0)
	,mRoot(0)
	,mFrameCounter(0)
	,mGameLogic(gameLogic)
	,mAutoUpdateTimer(0)
	,mSettings(0)
	,mAutoUpdateEnabled(true)
	,mIsInitialised(false)
	,mIgnoredConfigWarningMode(ignoredConfigWarningMode)
	{
		/*if(mGameLogic != 0)
		{
			mGameLogic->mApplication = this;
		}*/

		//Initialise the resources.
		//initQtResources();

		//Sanity check for config files
		if(mIgnoredConfigWarningMode == WarnAboutIgnoredConfigs)
		{
			QDirIterator it(".");
			while (it.hasNext())
			{
				it.next();
				if(QString::compare(it.fileInfo().suffix(), "cfg", Qt::CaseInsensitive) == 0)
				{
					if(	(QString::compare(it.fileInfo().baseName(), "plugins", Qt::CaseInsensitive) != 0) &&
						(QString::compare(it.fileInfo().baseName(), "plugins_d", Qt::CaseInsensitive) != 0) &&
						(QString::compare(it.fileInfo().baseName(), "resources", Qt::CaseInsensitive) != 0))
					{
						//We have found a file with the .cfg extension but which is not
						//'plugins.cfg' or 'resources.cfg'. Warn the user about this.
						warnAboutIgnoredConfigFile(it.fileInfo().fileName());
					}
				}
			}
		}

		mAutoUpdateTimer = new QTimer;
		QObject::connect(mAutoUpdateTimer, SIGNAL(timeout()), this, SLOT(update()));
		//On the test system, a value of one here gives a high frame rate and still allows
		//event processing to take place. A value of 0 doubles the frame rate but the mouse
		//becomes jumpy. This property is configerable via setAutoUpdateInterval().
		mAutoUpdateTimer->setInterval(1);

		//Load the settings file. If it doesn't exist it is created.
		mSettings = new QSettings("settings.ini", QSettings::IniFormat);

		mOgreWidget = new EventHandlingOgreWidget(0, 0);
		mOgreWidget->mApplication = this;

		//Logging should be initialised ASAP, and before Ogre::Root is created.
		initialiseLogging();

		//Create the Ogre::Root object.
		qDebug("Creating Ogre::Root object...");
		try
		{
#ifdef QT_DEBUG
			mRoot = new Ogre::Root("plugins_d.cfg");
#else
			mRoot = new Ogre::Root("plugins.cfg");
#endif
			qDebug("Ogre::Root created successfully.");
		}
		catch(Ogre::Exception& e)
		{
			QString error
				(
				"Failed to create the Ogre::Root object. This is a fatal error and the "
				"application will now exit. There are a few known reasons why this can occur:\n\n"
				"    1) Ensure your plugins.cfg has the correct path to the plugins.\n"
				"    2) In plugins.cfg, use unix style directorary serperators. I.e '/' rather than '\\'.\n"
				"    3) If your plugins.cfg is trying to load the Direct3D plugin, make sure you have DirectX installed on your machine.\n\n"
				"The message returned by Ogre was:\n\n"
				);
			error += QString::fromStdString(e.getFullDescription().c_str());

			qCritical(error.toAscii());
			showErrorMessageBox(error);

			//Not much else we can do here...
			std::exit(1);
		}

		//Load the render system plugins. We do that here so that we know what
		//render systems are available by the time we show the settings dialog.
		//Note that the render system is not initialised until the user selects one.
		mOpenGLRenderSystem = mRoot->getRenderSystemByName("OpenGL Rendering Subsystem");
		mDirect3D9RenderSystem = mRoot->getRenderSystemByName("Direct3D9 Rendering Subsystem");
		if(!(mOpenGLRenderSystem || mDirect3D9RenderSystem))
		{
			qCritical("No rendering subsystems found");
			showErrorMessageBox("No rendering subsystems found. Please ensure that your 'plugins.cfg' (or 'plugins_d.cfg') is correct and can be found by the executable.");
		}

		mSettingsDialog = new SettingsDialog(mSettings, mOgreWidget);
		mGraphicsSettingsWidget = new GraphicsSettingsWidget;
		mSettingsDialog->addSettingsWidget("Graphics", mGraphicsSettingsWidget);
	}

	Application::~Application()
	{
		if (mRoot)
		{
            delete mRoot;
			mRoot = 0;
		}

		//We delete the OgreWidget last because it
		//owns the LogManager (through Qt's mechanism).
		if(mOgreWidget)
		{
			delete mOgreWidget;
			mOgreWidget = 0;
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	/// \return whether the OpenGL render system is available
	////////////////////////////////////////////////////////////////////////////////
	bool Application::isOpenGLAvailable(void) const
	{
		return mOpenGLRenderSystem != 0;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// \return whether the Direct3D9 render system is available.
	////////////////////////////////////////////////////////////////////////////////
	bool Application::isDirect3D9Available(void) const
	{
		return mDirect3D9RenderSystem != 0;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// \return the number of frames rendered since the application started.
	////////////////////////////////////////////////////////////////////////////////
	unsigned int Application::frameCount(void) const
	{
		return mFrameCounter;
	}

	////////////////////////////////////////////////////////////////////////////////
	///This function is an implementation detail, and should not really be exposed.
	///It return the log which the QtOgre framework uses for its messages, whereas
	///users are expected to instead create their own log with createLog(). The reason
	///it is exposed is that the Qt debugging system (qtMessageHandler()) also redirects
	///to this log, and that cannot be made a member function.
	/// \return the log used by the QtOgre framework.
	////////////////////////////////////////////////////////////////////////////////
	Log* Application::_systemLog(void) const
	{
		return mSystemLog;
	}

	////////////////////////////////////////////////////////////////////////////////
	/// \return a pointer to the applications main window.
	////////////////////////////////////////////////////////////////////////////////
	QWidget* Application::mainWidget(void) const
	{
		return mOgreWidget;
	}

	void Application::applySettings(void)
	{
		//Eventually Application settings might be applied here.
		//Until then, we just pass the settings on the the MainWindow and GameLogic
		if(!mOgreWidget->applySettings(mSettingsDialog->mSettings))
		{
			showWarningMessageBox("Unable to apply desired settings to the window.\nPlease consult the system log for details");
		}

		/*if(mGameLogic != 0)
		{
			mGameLogic->applySettings(mSettings);
		}*/
	}

	void Application::initialise(void)
	{

		mOgreWidget->show();
		mOgreWidget->resize(800,600);
		centerWidget(mOgreWidget);

		centerWidget(mLogManager, mOgreWidget);

		mLogManager->setForceProcessEvents(true);
		initialiseOgre();
		
		
		mLogManager->setForceProcessEvents(false);

		//mLogManager->hide();

		//This is a bit of a hack, necessary because we want to use the settings dialog in two different
		//ways. The first time it is shown (by Application::exec()) no slot is connected - the Accepted
		//event is handled explicitly because the system is not initialised at that point. But now (and
		//when the dialog is shown in future) we are ready for it, so we connect the accepted() signal.
		//We also call accept, to do the initial setup. See also Application::exec().
		connect(mSettingsDialog, SIGNAL(accepted()), this, SLOT(applySettings()));
		mSettingsDialog->accept();

		Ogre::NameValuePairList ogreWindowParams;
		//ogreWindowParams["FSAA"] = "8";
		mOgreWidget->initialiseOgre(&ogreWindowParams);

		//Set up resource paths. This can't be done until the OgreWidget
		//is initialised, because we need the GPUProgramManager.
		if(QFile::exists("resources.cfg"))
		{
			loadResourcePathsFromConfigFile("resources.cfg");
			Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
		}

		mFPSDialog = new FPSDialog(mOgreWidget, Qt::ToolTip);
		mFPSDialog->setWindowOpacity(settings()->value("System/DefaultWindowOpacity", 1.0).toDouble());
		mFPSDialog->move(mainWidget()->geometry().topLeft() + QPoint(10,10));

		mLogManager->move(mainWidget()->geometry().left() + 10, mainWidget()->geometry().top() + mainWidget()->geometry().height() - mLogManager->frameGeometry().height() - 10);

		mOgreWidget->initialise();

		if(mAutoUpdateEnabled)
		{
			mAutoUpdateTimer->start();
		}

		mIsInitialised = true;
	}

	void Application::update(void)
	{
		//mGameLogic->update();
		mOgreWidget->update();
		++mFrameCounter;
	}

	void Application::shutdown(void)
	{
		mAutoUpdateTimer->stop();
		mOgreWidget->shutdown();
		mInternalOgreLog->removeListener(this);
		this->exit(0);
	}

	void Application::initialiseLogging(void)
	{
		//Initialise our logging system
		mLogManager = new LogManager(mOgreWidget);
		mLogManager->resize(550, 400);
		mLogManager->setWindowOpacity(settings()->value("System/DefaultWindowOpacity", 1.0).toDouble());

		//Redirect Qt's logging system to our own
		mSystemLog = mLogManager->createLog("System");
		qInstallMsgHandler(&qtMessageHandler);
		qDebug("***System log initialised***");

		//Redirect Ogre's logging system to our own
		//This has to all be done before creating the Root object.
		mOgreLog = mLogManager->createLog("Ogre");
		mLogManager->setVisibleLog(mOgreLog);
		mInternalOgreLogManager = new Ogre::LogManager();
		mInternalOgreLog = mInternalOgreLogManager->createLog("Ogre.log", false, true, true);
		mInternalOgreLog->addListener(this);
		mOgreLog->logMessage("***Ogre log initialised and redirected by QtOgre framework.***", LL_DEBUG);
		mOgreLog->logMessage("***Any further messages in this log come directly from Ogre.***", LL_DEBUG);
	}

	void Application::initialiseOgre(void)
	{
		QString renderSystem = mSettingsDialog->mSettings->value("Graphics/RenderSystem").toString();
		if(renderSystem.compare("OpenGL Rendering Subsystem") == 0)
		{
			mActiveRenderSystem = mOpenGLRenderSystem;
		}
		if(renderSystem.compare("Direct3D9 Rendering Subsystem") == 0)
		{
			mActiveRenderSystem = mDirect3D9RenderSystem;
		}

		Ogre::Root::getSingletonPtr()->setRenderSystem(mActiveRenderSystem);

		Ogre::Root::getSingletonPtr()->initialise(false);
	}

	void Application::messageLogged(const Ogre::String& message, Ogre::LogMessageLevel lml, bool maskDebug, const Ogre::String& logName)
	{
		//Convert message to Qt's string type.
		QString messageAsQString = QString::fromStdString(message);

		//Map Ogre's LogMessageLevels to our LogLevels
		switch(lml)
		{
		case Ogre::LML_TRIVIAL:
			mOgreLog->logMessage(messageAsQString, LL_DEBUG);
			break;
		case Ogre::LML_NORMAL:
			mOgreLog->logMessage(messageAsQString, LL_INFO);
			break;
		default: //Should be Ogre::LML_CRITICAL
			mOgreLog->logMessage(messageAsQString, LL_ERROR);
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	/// \param displaySettingsDialog should the settings dialog be displayed
	/// \return the application return code
	////////////////////////////////////////////////////////////////////////////////
	int Application::exec(SettingsDialogMode eDialogMode)
	{
		//If we don't show the setting dialog, or we do show it and it is accepted, then proceed.
		if((eDialogMode == SuppressSettingsDialog) || (qApp->showSettingsDialog() == QDialog::Accepted))
		{
			qApp->initialise();
			return QApplication::exec();
		}
		//Otherwise the user cancelled so exit.
		else
		{
			return 0;
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	/// Centers a widget inside its parent, or inside the desktop if no
	/// parent is provided. This requires the widget to be free to move.
	/// If not (e.g. its in a layout) then the behaviour is undefined
	/// \param widgetToCenter the widget to centre
	/// \param parent the parent of the widget
	////////////////////////////////////////////////////////////////////////////////	
	void Application::centerWidget(QWidget* widgetToCenter, QWidget* parent)
	{
		QRect parentGeometry;
		if(parent != 0)
		{
			parentGeometry = parent->frameGeometry();
		}
		else
		{
			parentGeometry = desktop()->availableGeometry();
		}

		int xOffset = (parentGeometry.width() - widgetToCenter->frameGeometry().width()) / 2;
		int yOffset = (parentGeometry.height() - widgetToCenter->frameGeometry().height()) / 2;
		widgetToCenter->move(parentGeometry.x() + xOffset, parentGeometry.y() + yOffset);
	}

	////////////////////////////////////////////////////////////////////////////////
	/// \param text the text to display
	////////////////////////////////////////////////////////////////////////////////
	void Application::showInfoMessageBox(const QString& text)
	{
		QMessageBox msgBox;
		msgBox.setWindowTitle("Information");
		msgBox.setIconPixmap(QPixmap(":/images/icons/dialog-information.svg"));
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setText(text);
		msgBox.exec();
	}

	////////////////////////////////////////////////////////////////////////////////
	/// \param text the text to display
	////////////////////////////////////////////////////////////////////////////////
	void Application::showWarningMessageBox(const QString& text)
	{
		QMessageBox msgBox;
		msgBox.setWindowTitle("Warning");
		msgBox.setIconPixmap(QPixmap(":/images/icons/dialog-warning.svg"));
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setText(text);
		msgBox.exec();
	}

	////////////////////////////////////////////////////////////////////////////////
	/// \param text the text to display
	////////////////////////////////////////////////////////////////////////////////
	void Application::showErrorMessageBox(const QString& text)
	{
		QMessageBox msgBox;
		msgBox.setWindowTitle("Error");
		msgBox.setIconPixmap(QPixmap(":/images/icons/dialog-error.svg"));
		msgBox.setStandardButtons(QMessageBox::Ok);
		msgBox.setText(text);
		msgBox.exec();
	}

	void Application::hideSettingsDialog(void)
	{
		mSettingsDialog->reject();
	}

	int Application::showSettingsDialog(void)
	{
		return mSettingsDialog->exec();
	}

	////////////////////////////////////////////////////////////////////////////////
	/// \param name the name of the log
	/// \return a pointer to the log
	////////////////////////////////////////////////////////////////////////////////
	Log* Application::createLog(const QString& name)
	{
		//Forward the request to the LogManager.
		return mLogManager->createLog(name);
	}

	Log* Application::getLogByName(const QString& name)
	{
		//Forward the request to the LogManager.
		return mLogManager->getLogByName(name);
	}

	void Application::hideLogManager(void)
	{
		mLogManager->setVisible(false);
	}

	void Application::showLogManager(void)
	{
		mLogManager->setVisible(true);
	}
	
	////////////////////////////////////////////////////////////////////////////////
	/// \param title the title for the settings widget tab
	/// \param settingsWidget the widget to add to the dialog
	//////////////////////////////////////////////////////////////////////////////// 
	void Application::addSettingsWidget(const QString& title, AbstractSettingsWidget* settingsWidget)
	{
		mSettingsDialog->addSettingsWidget(title, settingsWidget);
	}

	////////////////////////////////////////////////////////////////////////////////
	/// \return a pointer to the Ogre RenderWindow
	//////////////////////////////////////////////////////////////////////////////// 
	Ogre::RenderWindow* Application::ogreRenderWindow(void) const
	{
		return mOgreWidget->getOgreRenderWindow();
	}

	////////////////////////////////////////////////////////////////////////////////
	/// \return a pointer to the application settings
	////////////////////////////////////////////////////////////////////////////////
	QSettings* Application::settings(void) const
	{
		return mSettings;
	}

	/**
	* Sets the period between sucessive updates.
	* \param intervalInMilliseconds the period between sucessive updates
	*/
	void Application::setAutoUpdateInterval(int intervalInMilliseconds)
	{
		mAutoUpdateTimer->setInterval(intervalInMilliseconds);
	}

	void Application::hideFPSCounter(void)
	{
		mFPSDialog->setVisible(false);
	}

	void Application::showFPSCounter(void)
	{
		mFPSDialog->setVisible(true);
	}

	void Application::setAutoUpdateEnabled(bool autoUpdateEnabled)
	{
		mAutoUpdateEnabled = autoUpdateEnabled;

		//Only call start if the app is initialised, otherwise
		//the update() function might be using null pointers.
		if(mAutoUpdateEnabled && mIsInitialised)
		{
			mAutoUpdateTimer->start();
		}
		else
		{
			mAutoUpdateTimer->stop();
		}
	}

	void Application::warnAboutIgnoredConfigFile(const QString& filename)
	{
		QString message;
		message += "The file \'" + filename + "\' has been found in the applications working directory.";
		message += "\n\n";
		message += "The '.cfg' extension implies the file may usually be used by Ogre and/or the ExampleApplication framework. ";
		message += "However, the QtOgre framework currently only supports the 'plugins.cfg', 'plugins_d.cfg', and 'resources.cfg' files. ";
		message += "The file will be ignored by the QtOgre framework, and your application may not behave as expected. ";
		message += "\n\n";
		message += "If the file is being used by your application code, or by one of the plugins which you are loading, then you can suppress ";
		message += "this warning dialog box by passing the appropriate flags to the Application constructor. Please consult the documentation.";
		showWarningMessageBox(message);
	}

	//This function is based on code from the Ogre ExampleApplication.
	//It is not constrained by the Ogre licence (free for any use).
	void Application::loadResourcePathsFromConfigFile(const QString& filename)
    {
        // Load resource paths from config file
		Ogre::ConfigFile cf;
		cf.load(filename.toStdString());

        // Go through all sections & settings in the file
        Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();

        Ogre::String secName, typeName, archName;
        while (seci.hasMoreElements())
        {
            secName = seci.peekNextKey();
            Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
            Ogre::ConfigFile::SettingsMultiMap::iterator i;
            for (i = settings->begin(); i != settings->end(); ++i)
            {
                typeName = i->first;
                archName = i->second;
                Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                    archName, typeName, secName);
            }
        }
    }
}
