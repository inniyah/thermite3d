#ifndef THERMITE_CANNONCONTROLLER_H_
#define THERMITE_CANNONCONTROLLER_H_

#include "ui_CannonController.h"

#include <QDialog>
#include <QTime>
#include <QTimer>

class CannonController : public QDialog, private Ui::CannonController
{
	Q_OBJECT

public:
	CannonController(QWidget* parent = 0, Qt::WindowFlags f = 0);
};

#endif /*THERMITE_CANNONCONTROLLER_H_*/
