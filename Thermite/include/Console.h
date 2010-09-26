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

#ifndef THERMITE_CONSOLE_H_
#define THERMITE_CONSOLE_H_

#include "ui_Console.h"

#include "ThermiteForwardDeclarations.h"

class QScriptEngine;

namespace Thermite
{
	class Console : public QWidget, private Ui::Console
	{
		Q_OBJECT

	public:
		Console(QScriptEngine* scriptEngine, QWidget* parent = 0, Qt::WindowFlags f = 0 );

	private slots:
		void executeCommand(void);

	private:
		QScriptEngine* mScriptEngine;
	};
}

#endif //THERMITE_CONSOLE_H_
