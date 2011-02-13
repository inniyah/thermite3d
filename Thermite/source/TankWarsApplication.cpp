#include "TankWarsApplication.h"

#include "TankWarsViewWidget.h"

namespace Thermite
{
	TankWarsApplication::TankWarsApplication(int & argc, char ** argv)
		:Application(argc, argv)
		,mOgreWidget(0)
	{
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