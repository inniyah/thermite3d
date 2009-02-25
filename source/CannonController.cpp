#include "CannonController.h"

CannonController::CannonController(QWidget* parent, Qt::WindowFlags f)
	:QDialog(parent, f)
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