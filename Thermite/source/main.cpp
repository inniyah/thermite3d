#include "Application.h"
#include "ThermiteGameLogic.h"

#include <QPushButton>
#include <QIcon>

using namespace QtOgre;
using namespace Thermite;

int main(int argc, char *argv[])
{
	Application app(argc, argv, new ThermiteGameLogic);
	return app.exec(DisplaySettingsDialog);
}