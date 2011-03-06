#include "TankWarsApplication.h"

#include "TankWarsViewWidget.h"

#include <QDir>

namespace Thermite
{
	TankWarsApplication::TankWarsApplication(int & argc, char ** argv)
		:Application(argc, argv)
		,mOgreWidget(0)
	{
		QDir pathToResources("../../");
		if(!pathToResources.exists())
		{
			QString message("Path to resources does not exist: " + pathToResources.path());
			qApp->showErrorMessageBox(message);
		}

		//Initialise all resources		
		addResourceDirectory("./resources/");
		addResourceDirectory(pathToResources.path());
	}

	TankWarsApplication::~TankWarsApplication()
	{
		if(mOgreWidget)
		{
			delete mOgreWidget;
			mOgreWidget = 0;
		}
	}

	/*void TankWarsApplication::update(void)
	{
		if(mOgreWidget)
		{
			mOgreWidget->update();
		}

		Application::update();
	}*/

	void TankWarsApplication::shutdown(void)
	{
		mOgreWidget->shutdown();
	}
}