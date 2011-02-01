#ifndef QTOGRE_SETTINGSDIALOG_H_
#define QTOGRE_SETTINGSDIALOG_H_

#include "ui_SettingsDialog.h"

#include "AbstractSettingsWidget.h"

namespace QtOgre
{

	class SettingsDialog : public QDialog, private Ui::SettingsDialog
	{
		Q_OBJECT

	public:
		SettingsDialog(QSettings* settings, QWidget *parent = 0);

		void addSettingsWidget(const QString& title, AbstractSettingsWidget* settingsWidget);

		QSettings* mSettings;
	};
}

#endif /*QTOGRE_SETTINGSDIALOG_H_*/
