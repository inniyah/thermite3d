#include "TankWarsApplication.h"

#include "TankWarsViewWidget.h"

namespace Thermite
{
	TankWarsApplication::TankWarsApplication(int & argc, char ** argv)
		:Application(argc, argv)
		,mOgreWidget(0)
	{
		mOgreWidget = new TankWarsViewWidget(0, 0);

		Ogre::NameValuePairList ogreWindowParams;
		ogreWindowParams["FSAA"] = "8";
		mOgreWidget->initialiseOgre(&ogreWindowParams);

		mOgreWidget->initialise();

		mOgreWidget->show();
		mOgreWidget->resize(800,600);
		centerWidget(mOgreWidget);
	}

	TankWarsApplication::~TankWarsApplication()
	{
		if(mOgreWidget)
		{
			delete mOgreWidget;
			mOgreWidget = 0;
		}
	}

	void TankWarsApplication::update(void)
	{
		if(mOgreWidget)
		{
			mOgreWidget->update();
		}

		Application::update();
	}

	void TankWarsApplication::shutdown(void)
	{
		mOgreWidget->shutdown();
	}
}