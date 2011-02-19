#include "Light.h"

#include "Utility.h"

#include "OgreEntity.h"
#include "OgreRoot.h"

namespace Thermite
{
	Light::Light(Object* parent)
		:RenderComponent(parent)
		,m_colColour(255,255,255)
		,mType(PointLight)
		,mOgreLight(0)
	{
	}

	void Light::update(void)
	{
		RenderComponent::update();

		std::string objAddressAsString = QString::number(reinterpret_cast<qulonglong>(mParent), 16).toStdString();

		Ogre::Light* ogreLight;
		std::string lightName(objAddressAsString + "_Light");

		/*if(mOgreSceneManager->hasLight(lightName))
		{
			ogreLight = mOgreSceneManager->getLight(lightName);
		}
		else*/
		if(!mOgreLight)
		{
			Ogre::SceneManager* sceneManager = Ogre::Root::getSingletonPtr()->getSceneManager("OgreSceneManager");
			mOgreLight = sceneManager->createLight(lightName);
			Ogre::Entity* ogreEntity = sceneManager->createEntity(generateUID("PointLight Marker"), "sphere.mesh");
			mOgreSceneNode->attachObject(mOgreLight);
			mOgreSceneNode->attachObject(ogreEntity);
		}

		switch(getType())
		{
		case Light::PointLight:
			mOgreLight->setType(Ogre::Light::LT_POINT);
			break;
		case Light::DirectionalLight:
			mOgreLight->setType(Ogre::Light::LT_DIRECTIONAL);
			break;
		case Light::SpotLight:
			mOgreLight->setType(Ogre::Light::LT_SPOTLIGHT);
			break;
		}

		//Note we negate the z axis as Thermite considers negative z
		//to be forwards. This means that lights will match cameras.
		QVector3D dir = -mParent->zAxis();
		mOgreLight->setDirection(Ogre::Vector3(dir.x(), dir.y(), dir.z()));

		QColor col = getColour();
		mOgreLight->setDiffuseColour(col.redF(), col.greenF(), col.blueF());
	}

	const QColor& Light::getColour(void) const
	{
		return m_colColour;
	}

	void Light::setColour(const QColor& col)
	{
		m_colColour = col;
		mParent->setModified(true);
	}

	Light::LightType Light::getType(void) const
	{
		return mType;
	}

	void Light::setType(Light::LightType type)
	{
		mType = type;
		mParent->setModified(true);
	}
}