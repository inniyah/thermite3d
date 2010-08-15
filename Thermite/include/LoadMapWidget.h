/*******************************************************************************
Copyright (c) 2005-2009 David Williams

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution. 	
*******************************************************************************/

#ifndef THERMITE_LOADMAPWIDGET_H_
#define THERMITE_LOADMAPWIDGET_H_

#include "ui_LoadMapWidget.h"

#include <QDialog>
#include <QTime>
#include <QTimer>

#include "ThermiteForwardDeclarations.h"

namespace Thermite
{
	class LoadMapWidget : public QWidget, private Ui::LoadMapWidget
	{
		Q_OBJECT

	public:
		LoadMapWidget(ThermiteGameLogic* thermiteGameLogic, QWidget* parent = 0, Qt::WindowFlags f = 0 );

	public slots:
		void on_m_btnLoad_clicked(void);
		void on_m_btnClose_clicked(void);
		void on_m_listMap_currentTextChanged(const QString & currentText);

	private:
		QString m_strCurrentText;
		ThermiteGameLogic* m_thermiteGameLogic;
	};
}

#endif /*THERMITE_LOADMAPWIDGET_H_*/
