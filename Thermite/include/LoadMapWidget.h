#ifndef THERMITE_LOADMAPWIDGET_H_
#define THERMITE_LOADMAPWIDGET_H_

#include "ui_LoadMapWidget.h"

#include <QDialog>
#include <QTime>
#include <QTimer>

#include "ThermiteForwardDeclarations.h"

namespace Thermite
{
	class LoadMapWidget : public QWidget, private Ui::LoadMapWidget
	{
		Q_OBJECT

	public:
		LoadMapWidget(ThermiteGameLogic* thermiteGameLogic, QWidget* parent = 0, Qt::WindowFlags f = 0 );

	public slots:
		void on_m_btnLoad_clicked(void);

	private:
		ThermiteGameLogic* m_thermiteGameLogic;
	};
}

#endif /*THERMITE_LOADMAPWIDGET_H_*/