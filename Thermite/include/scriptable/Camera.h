#ifndef CAMERA_H_
#define CAMERA_H_

#include "RenderComponent.h"

#include <QVector3D>

namespace Thermite
{
	class Camera : public RenderComponent
	{
		Q_OBJECT

	public:
		Camera(Object* parent = 0);

		Q_PROPERTY(float fieldOfView READ fieldOfView WRITE setFieldOfView)

		float fieldOfView(void) const;
		void setFieldOfView(float fieldOfView);

	protected:
		float mFieldOfView;
	};
}

#endif //CAMERA_H_