#ifndef VECTOR3DPROTOTYPE_H
#define VECTOR3DPROTOTYPE_H

#include <QObject>
#include <QScriptable>
#include <QScriptValue>
#include <QTextStream>
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

	QString toString() const {QString result; QTextStream(&result) << "Vector3D(" << x() << "," << y() << "," << z() << ")"; return result;}

	//Member operators - can't easily modify 'thisVector3D()'?
	//QVector3D&	timesEquals ( qreal factor ) 
	//QVector3D&	timesEquals ( const QVector3D & vector )
	//QVector3D&	plusEquals ( const QVector3D & vector )
	//QVector3D&	minusEquals ( const QVector3D & vector )
	//QVector3D&	divideEquals ( qreal divisor )

	//Static methods - Search for 'static' in 'making applications scriptable'. It says:
	//"Functions that don't operate on the this object ("static" methods) are typically
	//stored as properties of the constructor function, not as properties of the prototype
	//object. The same is true for constants, such as enum values".
	//For now let's just make them normal methods
	QVector3D	crossProduct (const QVector3D & v2 ) {return QVector3D::crossProduct(*thisVector3D(), v2);}
	qreal	dotProduct (const QVector3D & v2 ) {return QVector3D::dotProduct(*thisVector3D(), v2);}
	QVector3D	normal (const QVector3D & v2 ) {return QVector3D::normal(*thisVector3D(), v2);}
	QVector3D	normal (const QVector3D & v2, const QVector3D & v3 ) {return QVector3D::normal(*thisVector3D(), v2, v3);}

	//Operator overloading not supported by QtScript
	bool qFuzzyCompare(const QVector3D & v2 ) {return ::qFuzzyCompare(*thisVector3D(), v2);}
	bool notEqualTo(const QVector3D & v2 ) {return *thisVector3D() != v2;}
	//const QVector3D	operator* ( qreal factor, const QVector3D & vector )
	const QVector3D	times(qreal factor ) {return *thisVector3D() * factor;}
	const QVector3D	times(const QVector3D & v2 ) {return *thisVector3D() * v2;}
	const QVector3D	plus(const QVector3D & v2 ) {return *thisVector3D() + v2;}
	const QVector3D	minus(const QVector3D & v2 ) {return *thisVector3D() - v2;}
	const QVector3D	negate() {return -(*thisVector3D());}
	const QVector3D	divideBy (qreal divisor ) {return *thisVector3D() / divisor;}
	//QDataStream &	operator<< ( QDataStream & stream, const QVector3D & vector )
	bool equalTo (const QVector3D & v2 ) {return *thisVector3D() == v2;}
	//QDataStream &	operator>> ( QDataStream & stream, QVector3D & vector )

private:
    QVector3D* thisVector3D() const;
};

#endif //VECTOR3DPROTOTYPE_H