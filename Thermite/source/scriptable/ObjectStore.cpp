#include "ObjectStore.h"

namespace Thermite
{
	Q_INVOKABLE QObject* ObjectStore::getObject(const QString& name)
	{
		return value(name);
	}

	Q_INVOKABLE void ObjectStore::setObject(const QString& name, QObject* object)
	{
		insert(name, object);
	}
}