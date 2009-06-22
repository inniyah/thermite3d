#include "MainMenu.h"

#include "Application.h"
#include "OgreWidget.h"
#include "SettingsDialog.h"

using namespace QtOgre;

namespace Thermite
{
	MainMenu::MainMenu(QWidget *parent, Qt::WindowFlags f)
	:QWidget(parent, f)
	{
		setupUi(this);
	}

	void MainMenu::on_mQuitButton_clicked(void)
	{
		emit quitClicked();
	}

	void MainMenu::on_mResumeButton_clicked(void)
	{
		//hide();
		emit resumeClicked();
	}

	void MainMenu::on_mSettingsButton_clicked(void)
	{
		//mApplication->showSettingsDialog();
		emit settingsClicked();
	}

	void MainMenu::on_mViewLogsButton_clicked(void)
	{
		emit viewLogsClicked();
		//mApplication->showLogManager();
		//reject();
	}

	void MainMenu::on_mLoadButton_clicked(void)
	{
		emit loadClicked();
		//mApplication->showLogManager();
		//reject();
	}
}