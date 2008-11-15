#ifndef DEMOGAMELOGIC_H_
#define DEMOGAMELOGIC_H_

#include "ChooseMeshWidget.h"
#include "GameLogic.h"
#include "MainMenu.h"

#include <OgrePrerequisites.h>

#include <QHash>

#include <QTime>

namespace QtOgre
{
	enum KeyStates
	{
		KS_RELEASED,
		KS_PRESSED
	};

	class DemoGameLogic : public GameLogic
	{
	public:
		DemoGameLogic(void);

		void initialise(void);
		void update(void);
		void shutdown(void);

		void onKeyPress(QKeyEvent* event);
		void onKeyRelease(QKeyEvent* event);

		void onMouseMove(QMouseEvent* event);
		void onMousePress(QMouseEvent* event);

		void onWheel(QWheelEvent* event);

		QtOgre::Log* demoLog(void);

	private:
		QHash<int, KeyStates> mKeyStates;
		QPoint mLastFrameMousePos;
		QPoint mCurrentMousePos;

		int mLastFrameWheelPos;
		int mCurrentWheelPos;
		QTime* mTime;

		int mLastFrameTime;
		int mCurrentTime;

		bool mIsFirstFrame;

		float mCameraSpeed;

		Ogre::Entity *mJaiquaEntity;
		Ogre::SceneNode* mJaiquaNode;

		Ogre::Entity *mRobotEntity;
		Ogre::SceneNode* mRobotNode;

		ChooseMeshWidget* mChooseMeshWidget;

		MainMenu* mMainMenu;

		Ogre::Camera* mCamera;
		Ogre::SceneManager* mSceneManager;
		QtOgre::Log* mDemoLog;
	};
}

#endif /*DEMOGAMELOGIC_H_*/