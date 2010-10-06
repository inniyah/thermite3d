#ifndef VECTOR3DCLASS_H
#define VECTOR3DCLASS_H

#include <QtCore/QObject>
#include <QtScript/QScriptClass>
#include <QtScript/QScriptString>

class Vector3DClass : public QObject, public QScriptClass
{
public:
    Vector3DClass(QScriptEngine *engine);
    ~Vector3DClass();

    QScriptValue constructor();

    QScriptValue newInstance();
    QScriptValue newInstance(const QVector3D &vec);

    QueryFlags queryProperty(const QScriptValue &object,
                            const QScriptString &name,
                            QueryFlags flags, uint *id);

    QScriptValue property(const QScriptValue &object,
                        const QScriptString &name, uint id);

    void setProperty(QScriptValue &object, const QScriptString &name,
                    uint id, const QScriptValue &value);

    QScriptValue::PropertyFlags propertyFlags(
        const QScriptValue &object, const QScriptString &name, uint id);

    QString name() const;

    QScriptValue prototype() const;

private:
    static QScriptValue construct(QScriptContext *ctx, QScriptEngine *eng);

    static QScriptValue toScriptValue(QScriptEngine *eng, const QVector3D &vec);
    static void fromScriptValue(const QScriptValue &obj, QVector3D &vec);

    QScriptString x;
	QScriptString y;
	QScriptString z;
    QScriptValue proto;
    QScriptValue ctor;
};

#endif //VECTOR3DCLASS_H
