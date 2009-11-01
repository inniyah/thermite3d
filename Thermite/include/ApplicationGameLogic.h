#pragma region License
/******************************************************************************
This file is part of the Thermite 3D game engine
Copyright (C) 2006  David Williams

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
******************************************************************************/
#pragma endregion

#ifndef __THERMITE_APPLICATIONGAMELOGIC_H__
#define __THERMITE_APPLICATIONGAMELOGIC_H__

#include "ThermiteGameLogic.h"

#include "ThermiteForwardDeclarations.h"

namespace Thermite
{
	class MainMenu;

	class ApplicationGameLogic : public ThermiteGameLogic
	{
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

		void fireCannon(void);

		void createSphereAt(PolyVox::Vector3DFloat centre, float radius, PolyVox::uint8_t value, bool bPaintMode);

	protected:
		//For keyboard handling
		QHash<int, KeyStates> mKeyStates;

		//For mouse buttons.
		Qt::MouseButtons mMouseButtonStates;
		
		//For mouse handling
		QPoint mLastFrameMousePos;
		QPoint mCurrentMousePos;
		int mLastFrameWheelPos;
		int mCurrentWheelPos;

		Ogre::Vector3 mCurrentMousePosInWorldSpace;

		float mCameraSpeed;
		float mCameraRotationalSpeed;

		MainMenu* mMainMenu;
		CannonController* mCannonController;

		Ogre::SceneNode* mTurretNode;
		Ogre::SceneNode* mGunNode;

		Ogre::Entity* mSphereBrush;
		Ogre::SceneNode* mSphereBrushNode;
		float mSphereBrushScale;
		PolyVox::uint8_t mSphereBrushMaterial;

		Ogre::Quaternion mTurretOriginalOrientation;
		Ogre::Quaternion mGunOriginalOrientation;

		std::list<Shell*> m_listShells;
	};
}

#endif //__THERMITE_APPLICATIONGAMELOGIC_H__