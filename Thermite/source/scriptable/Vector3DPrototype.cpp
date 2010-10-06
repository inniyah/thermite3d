#include "Vector3DPrototype.h"

#include <QScriptEngine>

Q_DECLARE_METATYPE(QVector3D*)

Vector3DPrototype::Vector3DPrototype(QObject *parent)
    : QObject(parent)
{
}

Vector3DPrototype::~Vector3DPrototype()
{
}

QVector3D* Vector3DPrototype::thisVector3D() const
{
    return qscriptvalue_cast<QVector3D*>(thisObject().data());
}

QScriptValue Vector3DPrototype::valueOf() const
{
    return thisObject().data();
}