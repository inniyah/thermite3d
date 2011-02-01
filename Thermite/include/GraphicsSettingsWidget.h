#ifndef QTOGRE_GRAPHICSSETTINGSWIDGET_H_
#define QTOGRE_GRAPHICSSETTINGSWIDGET_H_

#include "ui_GraphicsSettingsWidget.h"

#include "AbstractSettingsWidget.h"

namespace QtOgre
{

	class GraphicsSettingsWidget : public AbstractSettingsWidget, private Ui::GraphicsSettingsWidget
	{
		Q_OBJECT

	public:
		GraphicsSettingsWidget(QWidget *parent = 0);

		void disableFirstTimeOnlySettings(void);
		void readFromSettings(void);
		void writeToSettings(void);

	public slots:

		void on_mDirect3D9RadioButton_toggled(bool checked);
	};
}

#endif /*QTOGRE_GRAPHICSSETTINGSWIDGET_H_*/
