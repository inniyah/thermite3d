#include <QtScript/QScriptClassPropertyIterator>
#include <QtScript/QScriptEngine>
#include "Vector3DClass.h"
#include "Vector3DPrototype.h"

#include <stdlib.h>

Q_DECLARE_METATYPE(QVector3D*)
Q_DECLARE_METATYPE(Vector3DClass*)

Vector3DClass::Vector3DClass(QScriptEngine *engine)
    : QObject(engine), QScriptClass(engine)
{
    qScriptRegisterMetaType<QVector3D>(engine, toScriptValue, fromScriptValue);

    x = engine->toStringHandle(QLatin1String("x"));
	y = engine->toStringHandle(QLatin1String("y"));
	z = engine->toStringHandle(QLatin1String("z"));

    proto = engine->newQObject(new Vector3DPrototype(this),
                            QScriptEngine::QtOwnership,
                            QScriptEngine::SkipMethodsInEnumeration
                            | QScriptEngine::ExcludeSuperClassMethods
                            | QScriptEngine::ExcludeSuperClassProperties);
    QScriptValue global = engine->globalObject();
    proto.setPrototype(global.property("Object").property("prototype"));

    ctor = engine->newFunction(construct, proto);
    ctor.setData(qScriptValueFromValue(engine, this));
}

Vector3DClass::~Vector3DClass()
{
}

QScriptClass::QueryFlags Vector3DClass::queryProperty(const QScriptValue &object,
                                                    const QScriptString &name,
                                                    QueryFlags flags, uint *id)
{
    QVector3D *vec = qscriptvalue_cast<QVector3D*>(object.data());
    if (!vec)
	{
        return 0;
	}

    if ((name == x) || (name == y) || (name == z))
	{
        return flags;
    }

	return 0;
}

QScriptValue Vector3DClass::property(const QScriptValue &object,
                                    const QScriptString &name, uint id)
{
    QVector3D *vec = qscriptvalue_cast<QVector3D*>(object.data());
    if (!vec)
	{
        return QScriptValue();
	}

    if (name == x)
	{
        return vec->x();
    }
	else if (name == y)
	{
        return vec->y();
    } 
	else if (name == z)
	{
        return vec->z();
    } 

    return QScriptValue();
}

void Vector3DClass::setProperty(QScriptValue &object,
                                const QScriptString &name,
                                uint id, const QScriptValue &value)
{
    QVector3D *vec = qscriptvalue_cast<QVector3D*>(object.data());
    if (!vec)
	{
        return;
	}
    if (name == x) 
	{
		vec->setX(value.toNumber());
    }
	if (name == y) 
	{
		vec->setY(value.toNumber());
    }
	if (name == z) 
	{
		vec->setZ(value.toNumber());
    }
}

QScriptValue::PropertyFlags Vector3DClass::propertyFlags(
    const QScriptValue &/*object*/, const QScriptString &name, uint /*id*/)
{
	//All our properties are undeletable
    return QScriptValue::Undeletable;
}

QString Vector3DClass::name() const
{
    return QLatin1String("Vector3D");
}

QScriptValue Vector3DClass::prototype() const
{
    return proto;
}

QScriptValue Vector3DClass::constructor()
{
    return ctor;
}

QScriptValue Vector3DClass::newInstance()
{
    return newInstance(QVector3D());
}

QScriptValue Vector3DClass::newInstance(const QVector3D &vec)
{
    QScriptValue data = engine()->newVariant(qVariantFromValue(vec));
    return engine()->newObject(this, data);
}

QScriptValue Vector3DClass::construct(QScriptContext *ctx, QScriptEngine *)
{
    Vector3DClass *cls = qscriptvalue_cast<Vector3DClass*>(ctx->callee().data());
    if (!cls)
	{
        return QScriptValue();
	}

    QScriptValue arg = ctx->argument(0);
    if (arg.instanceOf(ctx->callee()))
        return cls->newInstance(qscriptvalue_cast<QVector3D>(arg));

	if(ctx->argumentCount() == 1)
	{
		QScriptValue arg = ctx->argument(0);
		QVector3D* vec = qscriptvalue_cast<QVector3D*>(arg);
		return cls->newInstance(*vec);
	}

	if(ctx->argumentCount() == 3)
	{
		QScriptValue xArg = ctx->argument(0);
		QScriptValue yArg = ctx->argument(1);
		QScriptValue zArg = ctx->argument(2);

		qreal x = xArg.toNumber();
		qreal y = yArg.toNumber();
		qreal z = zArg.toNumber();

		return cls->newInstance(QVector3D(x,y,z));
	}

    return cls->newInstance();
}

QScriptValue Vector3DClass::toScriptValue(QScriptEngine *eng, const QVector3D &vec)
{
    QScriptValue ctor = eng->globalObject().property("Vector3D");
    Vector3DClass *cls = qscriptvalue_cast<Vector3DClass*>(ctor.data());
    if (!cls)
        return eng->newVariant(qVariantFromValue(vec));
    return cls->newInstance(vec);
}

void Vector3DClass::fromScriptValue(const QScriptValue &obj, QVector3D &vec)
{
    vec = qvariant_cast<QVector3D>(obj.data().toVariant());
}
