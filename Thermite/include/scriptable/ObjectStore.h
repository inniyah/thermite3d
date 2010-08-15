#ifndef OBJECTSTORE_H_
#define OBJECTSTORE_H_

#include <QHash>
#include <QObject>

class ObjectStore : public QObject, public QHash<QString, QObject*>
{
	Q_OBJECT

//public slots:
public:
	Q_INVOKABLE QObject* getObject(const QString& name);

	Q_INVOKABLE void setObject(const QString& name, QObject* object);
};

#endif //OBJECTSTORE_H_