#include "CannonController.h"

#include "ApplicationGameLogic.h"

namespace Thermite
{
	CannonController::CannonController(ApplicationGameLogic* pApplicationGameLogic, QWidget* parent, Qt::WindowFlags f)
		:m_pApplicationGameLogic(pApplicationGameLogic)
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
		m_pApplicationGameLogic->fireCannon();
	}
}
