#ifndef THERMITE_CANNONCONTROLLER_H_
#define THERMITE_CANNONCONTROLLER_H_

#include "ui_CannonController.h"

#include <QDialog>
#include <QTime>
#include <QTimer>

namespace Thermite
{
	class ApplicationGameLogic;

	class CannonController : public QDialog, private Ui::CannonController
	{
		Q_OBJECT

	public:
		CannonController(ApplicationGameLogic* pApplicationGameLogic, QWidget* parent = 0, Qt::WindowFlags f = 0);

		int direction(void);
		int elevation(void);

		ApplicationGameLogic* m_pApplicationGameLogic;

	public slots:
			void on_fireButton_pressed(void);
	};
}

#endif /*THERMITE_CANNONCONTROLLER_H_*/
