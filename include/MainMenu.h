#ifndef MAINMENU_H_
#define MAINMENU_H_

#include "../ui_MainMenu.h"

#include "Application.h"

namespace QtOgre
{

	class MainMenu : public QDialog, private Ui::MainMenu
	{
		Q_OBJECT

	public:
		MainMenu(Application* application, QWidget* mainWidget, QWidget *parent = 0);

	public slots:
		void on_mQuitButton_clicked(void);
		void on_mResumeButton_clicked(void);
		void on_mSettingsButton_clicked(void);
		void on_mViewLogsButton_clicked(void);

	private:
		Application *mApplication;
		QWidget* mMainWidget;

	};
}

#endif /*MAINMENU_H_*/