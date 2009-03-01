#include "CannonController.h"

#include "ThermiteGameLogic.h"

namespace Thermite
{
	CannonController::CannonController(ThermiteGameLogic* pThermiteGameLogic, QWidget* parent, Qt::WindowFlags f)
		:m_pThermiteGameLogic(pThermiteGameLogic)
		,QDialog(parent, f)
	{
		setupUi(this);
	}

	int CannonController::direction(void)
	{
		return directionSlider->value();
	}

	int CannonController::elevation(void)
	{
		return elevationSlider->value();
	}

	void CannonController::on_fireButton_pressed(void)
	{
		m_pThermiteGameLogic->fireCannon();
	}
}
