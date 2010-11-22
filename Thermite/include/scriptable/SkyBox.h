#ifndef SKYBOX_H_
#define SKYBOX_H_

#include "Object.h"

#include <QScriptEngine>

namespace Thermite
{
	class SkyBox : public Object
	{
		Q_OBJECT

	public:		
		SkyBox(QObject* parent = 0);

		Q_PROPERTY(QString materialName READ materialName WRITE setMaterialName)

		const QString& materialName(void) const;
		void setMaterialName(const QString& name);

	private:
		QString mMaterialName;
	};
}

Q_SCRIPT_DECLARE_QMETAOBJECT(Thermite::SkyBox, QObject*)

#endif //SKYBOX_H_