#ifndef THERMITE_TANKWARSVIEWWIDGET_H_
#define THERMITE_TANKWARSVIEWWIDGET_H_

#include "ViewWidget.h"

class QWidget;

namespace Thermite
{

	class TankWarsViewWidget : public ViewWidget
	{
		Q_OBJECT

	public:
		TankWarsViewWidget(QWidget* parent=0, Qt::WindowFlags f=0);
		~TankWarsViewWidget();

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

		Object* fireballObject;
		Object* cursorObject;
		Object* cameraObject;
	};
}

#endif /*THERMITE_TANKWARSVIEWWIDGET_H_*/
