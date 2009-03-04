#ifndef SHELL_H_
#define SHELL_H_

#include "ThermiteForwardDeclarations.h"

#include "OgrePrerequisites.h"
#include "OgreVector3.h"

namespace Thermite
{
	class Shell
	{
	public:
		Shell(Map* pParentMap, Ogre::Vector3 vecPosition, Ogre::Vector3 vecVelocity);
		~Shell();

		void update(void);

		Ogre::Vector3 m_vecPosition;
		Ogre::Vector3 m_vecVelocity;
		Map* m_pParentMap;
		Ogre::SceneNode* m_pSceneNode;
		Ogre::Entity* m_pEntity;
	};
}

#endif /*SHELL_H_*/