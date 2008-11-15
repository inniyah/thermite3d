#include "MainMenu.h"

#include "Application.h"
#include "OgreWidget.h"
#include "SettingsDialog.h"

namespace QtOgre
{
	MainMenu::MainMenu(Application* application, QWidget* mainWidget, QWidget *parent)
	:QDialog(parent)
	{
		setupUi(this);

		mApplication = application;
		mMainWidget = mainWidget;
	}

	void MainMenu::on_mQuitButton_clicked(void)
	{
		mMainWidget->close();
	}

	void MainMenu::on_mResumeButton_clicked(void)
	{
		reject();
	}

	void MainMenu::on_mSettingsButton_clicked(void)
	{
		mApplication->showSettingsDialog();
	}

	void MainMenu::on_mViewLogsButton_clicked(void)
	{
		mApplication->showLogManager();
		//reject();
	}
}