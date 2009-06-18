#ifndef THERMITE_LOADINGPROGRESS_H_
#define THERMITE_LOADINGPROGRESS_H_

#include "ui_LoadingProgress.h"

#include <QDialog>
#include <QTime>
#include <QTimer>

#include "ThermiteForwardDeclarations.h"

namespace Thermite
{
	class LoadingProgress : public QWidget, private Ui::LoadingProgress
	{
		Q_OBJECT

	public:
		LoadingProgress(QWidget* parent = 0, Qt::WindowFlags f = 0 );

		void setLoadingDataPercentageDone(unsigned int percentageDone);
		void setExtractingSurfacePercentageDone(unsigned int percentageDone);
	};
}

#endif /*THERMITE_LOADINGPROGRESS_H_*/
