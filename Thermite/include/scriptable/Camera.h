#ifndef CAMERA_H_
#define CAMERA_H_

#include "Object.h"

#include <QScriptEngine>
#include <QVector3D>

class Camera : public Object
{
	Q_OBJECT

public:
	Camera(QObject* parent = 0);

	Q_PROPERTY(float fieldOfView READ fieldOfView WRITE setFieldOfView)

	float fieldOfView(void) const;
	void setFieldOfView(float fieldOfView);

protected:
	float mFieldOfView;
};

#endif //CAMERA_H_