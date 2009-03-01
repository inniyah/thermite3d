#include "Shell.h"

#include "Map.h"
#include "Utility.h"

namespace Thermite
{
	Shell::Shell(Map* pParentMap, Ogre::Vector3 vecPosition, Ogre::Vector3 vecVelocity)
	:m_pParentMap(pParentMap)
	,m_vecPosition(vecPosition)
	,m_vecVelocity(vecVelocity)
	{
		m_pSceneNode = m_pParentMap->m_pOgreSceneManager->getRootSceneNode()->createChildSceneNode(generateUID("ShellSceneNode"));
		m_pEntity = m_pParentMap->m_pOgreSceneManager->createEntity(generateUID("ShellEntity"), "Shell.mesh");
		m_pSceneNode->attachObject(m_pEntity);
		m_pSceneNode->setPosition(vecPosition);
	}

	void Shell::update(void)
	{
	}
}
