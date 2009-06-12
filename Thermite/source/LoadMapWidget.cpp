#include "LoadMapWidget.h"

#include "ThermiteGameLogic.h"

namespace Thermite
{
	LoadMapWidget::LoadMapWidget(ThermiteGameLogic* thermiteGameLogic, QWidget* parent, Qt::WindowFlags f)
		:QWidget(parent, f)
		,m_thermiteGameLogic(thermiteGameLogic)
	{
		setupUi(this);
	}

	void LoadMapWidget::on_m_btnLoad_clicked(void)
	{
		m_thermiteGameLogic->loadMap("dfsdfs");
	}
}
