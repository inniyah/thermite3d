#ifndef COMPONENT_H_
#define COMPONENT_H_

#include "Object.h"

#include <QObject>

namespace Thermite
{
	class Component : public QObject
	{
	public:
		Component(Object* parent);
		virtual ~Component(void);

		virtual void update(void);

		Object* mParent;
	};
}

#endif //COMPONENT_H_
