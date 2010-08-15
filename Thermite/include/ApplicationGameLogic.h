#pragma region License
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
#pragma endregion

#ifndef __THERMITE_APPLICATIONGAMELOGIC_H__
#define __THERMITE_APPLICATIONGAMELOGIC_H__

#include "ThermiteGameLogic.h"

#include "Camera.h"
#include "Entity.h"
#include "Globals.h"
#include "Keyboard.h"
#include "Light.h"
#include "Mouse.h"
#include "ObjectStore.h"
#include "ScriptEditorWidget.h"

#include <QtScript>
#include <QScriptEngineDebugger>

#include "ThermiteForwardDeclarations.h"

#include "OgreVector3.h"
#include "OgreQuaternion.h"

namespace Thermite
{
	class MainMenu;

	class ApplicationGameLogic : public ThermiteGameLogic
	{
		Q_OBJECT
	public:
		ApplicationGameLogic(void);

		void initialise(void);
		void update(void);

		void onKeyPress(QKeyEvent* event);
		void onKeyRelease(QKeyEvent* event);

		void onMouseMove(QMouseEvent* event);
		void onMousePress(QMouseEvent* event);
		void onMouseRelease(QMouseEvent* event);

		void onWheel(QWheelEvent* event);

		void onLoadMapClicked(QString strMapName);

		void createSphereAt(PolyVox::Vector3DFloat centre, float radius, uint8_t value, bool bPaintMode);

		void initScriptEngine(void);
		void initScriptEnvironment(void);

	private slots:
		void startScriptingEngine(void);
		void stopScriptingEngine(void);

	protected:
		//For keyboard handling
		/*QHash<int, KeyStates> mKeyStates;

		//For mouse buttons.
		Qt::MouseButtons mMouseButtonStates;
		
		//For mouse handling
		QPoint mLastFrameMousePos;
		QPoint mCurrentMousePos;
		int mLastFrameWheelPos;
		int mCurrentWheelPos;*/

		float mCameraSpeed;
		float mCameraRotationalSpeed;

		MainMenu* mMainMenu;

		Keyboard keyboard;
		Mouse* mouse;

		//Scripting
		QScriptEngine* scriptEngine;

		Camera* camera;

		QScriptEngineDebugger debugger;

		ScriptEditorWidget* m_pScriptEditorWidget;

		bool m_bRunScript;

		QHash<QString, Light*> m_Lights;

		ObjectStore mObjectStore;

		QString mInitialiseScript;

		Globals* mGlobals;
	};
}

#endif //__THERMITE_APPLICATIONGAMELOGIC_H__