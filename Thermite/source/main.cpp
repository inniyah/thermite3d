#include "Application.h"
#include "ApplicationGameLogic.h"

#include <QPushButton>
#include <QIcon>

using namespace QtOgre;
using namespace Thermite;

int main(int argc, char *argv[])
{
	Application app(argc, argv, new ApplicationGameLogic);
	return app.exec(DisplaySettingsDialog);
}