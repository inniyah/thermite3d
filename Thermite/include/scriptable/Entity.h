#ifndef ENTITY_H_
#define ENTITY_H_

#include "Object.h"

#include <QString>

namespace Thermite
{
	class Entity : public Object
	{
		Q_OBJECT

	public:
		Entity(QObject* parent = 0);

		Q_PROPERTY(QString meshName READ meshName WRITE setMeshName)
		Q_PROPERTY(QString materialName READ materialName WRITE setMaterialName)
		Q_PROPERTY(bool animated READ animated WRITE setAnimated)
		Q_PROPERTY(QString animationName READ animationName WRITE setAnimationName)
		Q_PROPERTY(bool loopAnimation READ loopAnimation WRITE setLoopAnimation)

		const QString& meshName(void) const;
		void setMeshName(const QString& name);

		const QString& materialName(void) const;
		void setMaterialName(const QString& name);

		const bool animated(void) const;
		void setAnimated(bool animated);

		const QString& animationName(void) const;
		void setAnimationName(const QString& name);

		const bool loopAnimation(void) const;
		void setLoopAnimation(bool loopAnimation);

	private:
		QString mMeshName;
		QString mMaterialName;

		bool mAnimated;
		QString mAnimationName;
		bool mLoopAnimation;
	};
}


#endif //ENTITY_H_