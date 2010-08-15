#include "Camera.h"

Camera::Camera(QObject * parent)
	:Object(parent)
{
	mFieldOfView = 1.0;
}

float Camera::fieldOfView(void) const
{
	return mFieldOfView;
}

void Camera::setFieldOfView(float fieldOfView)
{
	mFieldOfView = fieldOfView;
}