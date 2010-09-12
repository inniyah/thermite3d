#include "Object.h"

#include <math.h>

namespace Thermite
{
	Object::Object(QObject * parent)
		:QObject(parent)
	{
		mScale = QVector3D(1,1,1);
	}

	const QVector3D& Object::position(void) const
	{
		return mPosition;
	}

	void Object::setPosition(const QVector3D& position)
	{
		mPosition = position;
	}

	const QQuaternion& Object::orientation(void) const
	{
		return mOrientation;
	}

	void Object::setOrientation(const QQuaternion& orientation)
	{
		mOrientation = orientation;
	}

	const QVector3D& Object::size(void) const
	{
		return mScale;
	}

	void Object::setSize(const QVector3D& scale)
	{
		mScale = scale;
	}

	const QVector3D Object::xAxis(void) const
	{
		QVector3D axis(1,0,0);
		return mOrientation.rotatedVector(axis);
	}

	const QVector3D Object::yAxis(void) const
	{
		QVector3D axis(0,1,0);
		return mOrientation.rotatedVector(axis);
	}

	const QVector3D Object::zAxis(void) const
	{
		QVector3D axis(0,0,1);
		return mOrientation.rotatedVector(axis);
	}

	void Object::translate(const QVector3D & vector)
	{
		mPosition += vector;
	}

	void Object::translate(qreal x, qreal y, qreal z)
	{
		mPosition += QVector3D(x,y,z);
	}

	void Object::pitch(qreal angleInDegrees)
	{

		QQuaternion rotation = QQuaternion::fromAxisAndAngle(QVector3D(1,0,0), angleInDegrees);
		mOrientation *= rotation;
	}

	void Object::yaw(qreal angleInDegrees)
	{
		QQuaternion rotation = QQuaternion::fromAxisAndAngle(QVector3D(0,1,0), angleInDegrees);
		mOrientation *= rotation;
	}

	void Object::roll(qreal angleInDegrees)
	{
		QQuaternion rotation = QQuaternion::fromAxisAndAngle(QVector3D(0,0,1), angleInDegrees);
		mOrientation *= rotation;
	}

	void Object::scale(qreal factor)
	{
		mScale *= QVector3D(factor,factor,factor);
	}

	void Object::scale(const QVector3D & vector)
	{
		mScale *= vector;
	}

	void Object::scale(qreal x, qreal y, qreal z)
	{
		mScale *= QVector3D(x,y,z);
	}

	// This function rotates the object so that its *negative* z axis points at the target.
	// The negative z is used so that it works with cameras, which have z going *into* the screen.
	void Object::lookAt(const QVector3D& target)
	{
		//Our local negative z vector.
		QVector3D negativeZ(0.0f, 0.0f, -1.0f);

		//The point we want to look at.
		QVector3D desiredDirection = target - mPosition;
		desiredDirection.normalize();

		//The axis we need to rotate around.
		QVector3D axis = QVector3D::crossProduct(negativeZ, desiredDirection);

		//And the angle of rotation
		float angle = QVector3D::dotProduct(negativeZ, desiredDirection);
		angle = acosf(angle);
		angle *= 57.2957795f; //Convert to degrees.

		//Set the orientation.
		mOrientation = QQuaternion::fromAxisAndAngle(axis, angle);
	}
}
