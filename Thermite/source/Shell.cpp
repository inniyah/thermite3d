#include "Shell.h"

#include "Map.h"
#include "Utility.h"

namespace Thermite
{
	Shell::Shell(Map* pParentMap, Ogre::Vector3 vecPosition, Ogre::Vector3 vecVelocity)
	:m_pParentMap(pParentMap)
	,m_vecVelocity(vecVelocity)
	{
		m_pSceneNode = m_pParentMap->m_pOgreSceneManager->getRootSceneNode()->createChildSceneNode(generateUID("ShellSceneNode"));
		m_pEntity = m_pParentMap->m_pOgreSceneManager->createEntity(generateUID("ShellEntity"), "Shell.mesh");
		m_pSceneNode->attachObject(m_pEntity);
		m_pSceneNode->setPosition(vecPosition);

		//Tempoary hack so shell doesn't immediatly explode in the cannon
		m_pSceneNode->setPosition(vecPosition + vecVelocity.normalisedCopy());
	}

	Shell::~Shell()
	{
		m_pParentMap->m_pOgreSceneManager->getRootSceneNode()->removeChild(m_pSceneNode);
	}

	void Shell::update(float fTimeElapsedInSeconds)
	{
		float fAirResistance = 0.01f;
		Ogre::Vector3 vecGravity(0.0,-50.0,0.0);

		//Compute air resistance
		Ogre::Vector3 vecAirResistance = (-m_vecVelocity) * fAirResistance;

		//Apply air resistance and gravity - allowing for time elapsed
		m_vecVelocity += vecAirResistance * fTimeElapsedInSeconds;
		m_vecVelocity += vecGravity * fTimeElapsedInSeconds;

		//Move position by elocity - allowing for time elapsed
		Ogre::Vector3 vecNewPosition = m_pSceneNode->getPosition() + (m_vecVelocity * fTimeElapsedInSeconds);
		m_pSceneNode->setPosition(vecNewPosition);

		//Rotate the shell to point in the direction of travel
		m_pSceneNode->lookAt(vecNewPosition + m_vecVelocity, Ogre::Node::TS_PARENT, Ogre::Vector3::UNIT_Z);
	}
}
