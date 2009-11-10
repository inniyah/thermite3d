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

		void update(float fTimeElapsedInSeconds);

		Ogre::Vector3 m_vecVelocity;
		Map* m_pParentMap;
		Ogre::SceneNode* m_pSceneNode;
		Ogre::Entity* m_pEntity;
	};
}

#endif /*SHELL_H_*/