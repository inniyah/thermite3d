#include "Application.h"
#include "DemoGameLogic.h"

#include <QPushButton>
#include <QIcon>

using namespace QtOgre;

int main(int argc, char *argv[])
{
	Application app(argc, argv, new DemoGameLogic);
	return app.exec(true);
}