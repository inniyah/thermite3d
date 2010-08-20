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

#ifndef MAINMENU_H_
#define MAINMENU_H_

#include "../ui_MainMenu.h"

#include "Application.h"

namespace Thermite
{
	class MainMenu : public QWidget, private Ui::MainMenu
	{
		Q_OBJECT

	public:
		MainMenu(QWidget *parent = 0, Qt::WindowFlags f = 0);

		//Tempoary hack until reloading is fixed...
		void disableLoadButton(void) {mLoadButton->setEnabled(false);}

	public slots:
		void on_mQuitButton_clicked(void);
		void on_mResumeButton_clicked(void);
		void on_mSettingsButton_clicked(void);
		void on_mViewLogsButton_clicked(void);
		void on_mLoadButton_clicked(void);

	signals:
		void quitClicked(void);
		void resumeClicked(void);
		void settingsClicked(void);
		void viewLogsClicked(void);
		void loadClicked(void);
	};
}

#endif /*MAINMENU_H_*/