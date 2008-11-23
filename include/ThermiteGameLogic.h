#ifndef THERMITEGAMELOGIC_H_
#define THERMITEGAMELOGIC_H_

#include "GameLogic.h"
#include "MainMenu.h"

#include "PhysicalObject.h"
#include "World.h"

#include <OgrePrerequisites.h>

#include <QHash>

#include <QTime>

#include <queue>

namespace QtOgre
{
	enum KeyStates
	{
		KS_RELEASED,
		KS_PRESSED
	};

	class ThermiteGameLogic : public GameLogic
	{
	public:
		ThermiteGameLogic(void);

		void initialise(void);
		void update(void);
		void shutdown(void);

		void onKeyPress(QKeyEvent* event);
		void onKeyRelease(QKeyEvent* event);

		void onMouseMove(QMouseEvent* event);
		void onMousePress(QMouseEvent* event);

		void onWheel(QWheelEvent* event);

		QtOgre::Log* thermiteLog(void);

	private:
		void addResourceDirectory(const QString& directoryName);

		void createCube(float xPos, float zPos);

		QHash<int, KeyStates> mKeyStates;
		QPoint mLastFrameMousePos;
		QPoint mCurrentMousePos;

		int mLastFrameWheelPos;
		int mCurrentWheelPos;
		QTime* mTime;

		int mLastFrameTime;
		int mCurrentTime;

		unsigned int mCurrentFrameNumber;

		float mCameraSpeed;
		float mCameraRotationalSpeed;

		Ogre::Entity *mJaiquaEntity;
		Ogre::SceneNode* mJaiquaNode;

		MainMenu* mMainMenu;

		Ogre::Camera* mCamera;
		Ogre::SceneManager* mSceneManager;
		QtOgre::Log* mThermiteLog;

		//Thermite stuff
		World* mWorld;
		int cubeCounter; //For unique names

		std::queue<PhysicalObject*> m_queueObjects;

		Ogre::Light* light1;
		Ogre::SceneNode* lightNode1;
	};
}

#endif /*DEMOGAMELOGIC_H_*/