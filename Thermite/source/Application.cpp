#include "Application.h"

#include "TankWarsViewWidget.h"
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

namespace Thermite
{
	Application::Application(int& argc, char** argv, GameLogic* gameLogic)
	:QApplication(argc, argv)
	,mOgreWidget(0)
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
	{
		mAutoUpdateTimer = new QTimer;
		QObject::connect(mAutoUpdateTimer, SIGNAL(timeout()), this, SLOT(update()));
		//On the test system, a value of one here gives a high frame rate and still allows
		//event processing to take place. A value of 0 doubles the frame rate but the mouse
		//becomes jumpy. This property is configerable via setAutoUpdateInterval().
		mAutoUpdateTimer->setInterval(1);

		//Load the settings file. If it doesn't exist it is created.
		mSettings = new QSettings("settings.ini", QSettings::IniFormat);

		mOgreWidget = new TankWarsViewWidget(0, 0);

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
		if(!mOgreWidget->applySettings(mSettings))
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

		initialiseOgre();

		Ogre::NameValuePairList ogreWindowParams;
		//ogreWindowParams["FSAA"] = "8";
		mOgreWidget->initialiseOgre(&ogreWindowParams);

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
		this->exit(0);
	}

	void Application::initialiseOgre(void)
	{
		QString renderSystem = mSettings->value("Graphics/RenderSystem").toString();
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
}
