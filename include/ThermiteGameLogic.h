#ifndef THERMITEGAMELOGIC_H_
#define THERMITEGAMELOGIC_H_

#include "CannonController.h"
#include "GameLogic.h"
#include "MainMenu.h"

#include "Map.h"
#include "PhysicalEntity.h"

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
		CannonController* mCannonController;

		Ogre::Camera* mCamera;
		Ogre::SceneManager* mSceneManager;
		QtOgre::Log* mThermiteLog;

		//Thermite stuff
		Map* mMap;
		int cubeCounter; //For unique names

		std::queue<PhysicalObject*> m_queueObjects;

		//Ogre::Light* light1;
		//Ogre::SceneNode* lightNode1;
		Ogre::Entity* mCannon;
		Ogre::SceneNode* mCannonNode;
	};
}

#endif /*DEMOGAMELOGIC_H_*/