#ifndef VECTOR3DPROTOTYPE_H
#define VECTOR3DPROTOTYPE_H

#include <QObject>
#include <QScriptable>
#include <QScriptValue>
#include <QVector3D>

class Vector3DPrototype : public QObject, public QScriptable
{
Q_OBJECT

public:
    Vector3DPrototype(QObject *parent = 0);
    ~Vector3DPrototype();

    QScriptValue valueOf() const;

public slots:
	//Exposing stuff from QVector3D
	qreal distanceToLine ( const QVector3D & point, const QVector3D & direction ) const {return thisVector3D()->distanceToLine(point, direction);}
	qreal distanceToPlane ( const QVector3D & plane, const QVector3D & normal ) const {return thisVector3D()->distanceToPlane(plane, normal);}
	qreal distanceToPlane ( const QVector3D & plane1, const QVector3D & plane2, const QVector3D & plane3 ) const {return thisVector3D()->distanceToPlane(plane1, plane2);}
	bool isNull () const {return thisVector3D()->isNull();}
	qreal length () const {return thisVector3D()->length();}
	qreal lengthSquared () const {return thisVector3D()->lengthSquared();}
	void normalize () {thisVector3D()->normalize();}
	QVector3D normalized () const {return thisVector3D()->normalized();}
	void setX ( qreal x ) {thisVector3D()->setX(x);}
	void setY ( qreal y ) {thisVector3D()->setY(y);}
	void setZ ( qreal z ) {thisVector3D()->setZ(z);}
	//QPoint toPoint () const
	//QPointF toPointF () const
	//QVector2D toVector2D () const
	//QVector4D toVector4D () const
	qreal x () const {return thisVector3D()->x();}
	qreal y () const {return thisVector3D()->y();}
	qreal z () const {return thisVector3D()->z();}

private:
    QVector3D* thisVector3D() const;
};

#endif //VECTOR3DPROTOTYPE_H