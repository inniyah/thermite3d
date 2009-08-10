#ifndef THERMITEGAMELOGIC_H_
#define THERMITEGAMELOGIC_H_

#include "CannonController.h"
#include "GameLogic.h"
#include "MainMenu.h"
#include "LoadingProgress.h"

#include "Serialization.h"

#include "Map.h"
#include "PhysicalEntity.h"
#include "ThermiteForwardDeclarations.h"

#include <OgrePrerequisites.h>

#include <QHash>

#include <QTime>

#include <QMovie>
#include <QLabel>

#include <list>
#include <queue>

namespace Thermite
{
	enum KeyStates
	{
		KS_RELEASED,
		KS_PRESSED
	};

	class ThermiteGameLogic : public QtOgre::GameLogic
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

		void fireCannon(void);

		void createSphereAt(PolyVox::Vector3DFloat centre, float radius, PolyVox::uint8_t value);

		void loadMap(QString strMapName);

		void setVolumeLoadProgress(float fProgress);

		void reloadShaders(void);

	public:
		void addResourceDirectory(const QString& directoryName);

		void createCube(float xPos, float zPos);

		void createAxis(unsigned int uWidth, unsigned int uHeight, unsigned int uDepth);

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
		MoviePlayer* mMoviePlayer;

		Ogre::Camera* mCamera;
		Ogre::SceneManager* mSceneManager;
		QtOgre::Log* mThermiteLog;

		//Thermite stuff
		Map* mMap;
		int cubeCounter; //For unique names

		std::queue<PhysicalObject*> m_queueObjects;

		//Ogre::Entity* mCannon;
		Ogre::SceneNode* mTurretNode;
		Ogre::SceneNode* mGunNode;

		Ogre::Quaternion mTurretOriginalOrientation;
		Ogre::Quaternion mGunOriginalOrientation;

		std::list<Shell*> m_listShells;

		LoadingProgress* m_loadingProgress;

		QMovie* m_pThermiteLogoMovie;
		QLabel* m_pThermiteLogoLabel;

		Ogre::SceneNode* m_axisNode;

		bool bLoadComplete;

		QString m_strAppName;
	};
}

#endif /*DEMOGAMELOGIC_H_*/