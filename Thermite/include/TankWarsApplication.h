#ifndef TANKWARS_APPLICATION_H_
#define TANKWARS_APPLICATION_H_

#include "Application.h"

namespace Thermite
{
	class TankWarsApplication : public Application
	{
	public:
		TankWarsApplication(int & argc, char ** argv);
		~TankWarsApplication();

		void update(void);
		void shutdown(void);

	private:
		TankWarsViewWidget* mOgreWidget;
	};
}

#endif //TANKWARS_APPLICATION_H_