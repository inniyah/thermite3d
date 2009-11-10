/*******************************************************************************
Copyright (c) 2005-2009 David Williams

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution. 	
*******************************************************************************/

#include "Shell.h"

#include "Map.h"
#include "Utility.h"

#include "OgreEntity.h"
#include "OgreSceneManager.h"

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
		m_pSceneNode->setScale(Ogre::Vector3(0.25,0.25,0.25));

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
		Ogre::Vector3 vecGravity(0.0,-20.0,0.0);

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
