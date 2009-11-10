/*******************************************************************************
Copyright (c) 2005-2009 David Williams

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution. 	
*******************************************************************************/

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