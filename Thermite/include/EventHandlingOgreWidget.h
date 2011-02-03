#ifndef THERMITE_EVENTHANDLINGOGREWIDGET_H_
#define THERMITE_EVENTHANDLINGOGREWIDGET_H_

#include "OgreWidget.h"

#include "Serialization.h"

#include "Camera.h"
#include "Entity.h"
#include "Globals.h"
#include "Light.h"

#include "scriptable/Volume.h"
#include "ThermiteForwardDeclarations.h"

#include "QtOgreForwardDeclarations.h"

#include <OgrePrerequisites.h>
#include <OgreTexture.h>

#include <QHash>

#include <QTime>

#include <QMovie>
#include <QLabel>
#include <QObject>

#include <list>
#include <queue>

class QWidget;

namespace Thermite
{

	class EventHandlingOgreWidget : public OgreWidget
	{
		Q_OBJECT

	public:
		EventHandlingOgreWidget(QWidget* parent=0, Qt::WindowFlags f=0);
		~EventHandlingOgreWidget();

		void initialise(void);
		void update(void);
		void shutdown(void);

		void closeEvent(QCloseEvent *event);

		void keyPressEvent(QKeyEvent* event);
		void keyReleaseEvent(QKeyEvent* event);

		void mousePressEvent(QMouseEvent* event);
		void mouseReleaseEvent(QMouseEvent* event);
		void mouseDoubleClickEvent(QMouseEvent* event);
		void mouseMoveEvent(QMouseEvent* event);

		void wheelEvent(QWheelEvent* event);

	public:
		void addResourceDirectory(const QString& directoryName);

		void createAxis(void);

	public slots:
		void uploadSurfaceMesh(const PolyVox::SurfaceMesh<PolyVox::PositionMaterial>& mesh, PolyVox::Region region, Volume& volume);		
		void addSurfacePatchRenderable(std::string materialName, PolyVox::SurfaceMesh<PolyVox::PositionMaterial>& mesh, PolyVox::Region region);

		void uploadSurfaceMesh(const PolyVox::SurfaceMesh<PolyVox::PositionMaterialNormal>& mesh, PolyVox::Region region, Volume& volume);		
		void addSurfacePatchRenderable(std::string materialName, PolyVox::SurfaceMesh<PolyVox::PositionMaterialNormal>& mesh, PolyVox::Region region);

		bool loadApp(const QString& appName);
		void unloadApp(void);

		//Don't like having these functions here - really they should be inside camera or something. But they are
		//using Ogre methods which aren't available in the scriptable classes. Eventually they should be rewritten
		//without the utility classes. Also, there are two seperate functions because I couldn't pass QVector3D
		//by reference. Need to look at registering this type...
		QVector3D getPickingRayOrigin(int x, int y);
		QVector3D getPickingRayDir(int x, int y);

	private slots:

		void playStartupMovie(void);
		void showLastMovieFrame(void);
		void deleteMovie(void);

	public:
		//Deletes all children (both nodes and attached objects) but not the node itself.
		void deleteSceneNodeChildren(Ogre::SceneNode* sceneNode);

		void initialiseHandler(void);
		void updateHandler(void);

		//Scene representation
		Camera* mCamera;
		SkyBox* mSkyBox;
		PolyVox::Array<3, uint32_t> mVolLastUploadedTimeStamps;
		PolyVox::Array<3, uint32_t> mVolLightingLastUploadedTimeStamps;
		QList<Object*> mObjectList;
		uint16_t mCachedVolumeWidthInRegions;
		uint16_t mCachedVolumeHeightInRegions;
		uint16_t mCachedVolumeDepthInRegions;

		uint16_t mCachedVolumeWidthInLightRegions;
		uint16_t mCachedVolumeHeightInLightRegions;
		uint16_t mCachedVolumeDepthInLightRegions;

		Ogre::TexturePtr mAmbientOcclusionVolumeTexture;

		//Ogre's scene representation
		Ogre::SceneNode* mVolumeSceneNode;
		PolyVox::Array<3, Ogre::SceneNode*> m_volOgreSceneNodes;
		Ogre::SceneNode* mCameraSceneNode;
		Ogre::Camera* mOgreCamera;
		Ogre::Viewport* mMainViewport;
		Ogre::SceneManager* mOgreSceneManager;
		Ogre::SceneNode* m_axisNode;

		//Input
		Keyboard* keyboard;
		Mouse* mouse;

		//User interface
		QMovie* m_pThermiteLogoMovie;
		QLabel* m_pThermiteLogoLabel;

		//Other
		bool mFirstFind;

		//Game specific
		Object* cameraNode;
		float cameraSpeedInUnitsPerSecond;
		QVector3D cameraFocusPoint;
		float cameraElevationAngle;
		float cameraRotationAngle;
		float cameraDistance;

		Volume* volume;
		Entity* cursor;
		Entity* mMissile;
		Light* light0;
		Entity* fireball;
		float explosionSize;
		float explosionStartTime;
		float explosionAge;

		float currentTimeInSeconds;
		float timeElapsedInSeconds;
		float previousTimeInMS;
	};
}

#endif /*THERMITE_EVENTHANDLINGOGREWIDGET_H_*/
