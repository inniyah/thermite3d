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