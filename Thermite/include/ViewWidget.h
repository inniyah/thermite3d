#ifndef THERMITE_VIEWWIDGET_H_
#define THERMITE_VIEWWIDGET_H_

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

	class ViewWidget : public OgreWidget
	{
		Q_OBJECT

	public:
		ViewWidget(QWidget* parent=0, Qt::WindowFlags f=0);
		~ViewWidget();

		virtual void initialise(void);
		virtual void update(void);
		virtual void shutdown(void);

	public:
		void addResourceDirectory(const QString& directoryName);

		void createAxis(void);

	public slots:
		//void uploadSurfaceMesh(const PolyVox::SurfaceMesh<PolyVox::PositionMaterial>& mesh, PolyVox::Region region, Volume& volume);		
		//void addSurfacePatchRenderable(std::string materialName, PolyVox::SurfaceMesh<PolyVox::PositionMaterial>& mesh, PolyVox::Region region);

		//void uploadSurfaceMesh(const PolyVox::SurfaceMesh<PolyVox::PositionMaterialNormal>& mesh, PolyVox::Region region, Volume& volume);		
		//void addSurfacePatchRenderable(std::string materialName, PolyVox::SurfaceMesh<PolyVox::PositionMaterialNormal>& mesh, PolyVox::Region region);

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

		//Scene representation
		Camera* mCamera;
		//SkyBox* mSkyBox;
		

		Volume* mVolume;

		//Ogre's scene representation
		
		
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
	};
}

#endif /*THERMITE_VIEWWIDGET_H_*/
