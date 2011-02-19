#ifndef RENDER_COMPONENT_H_
#define RENDER_COMPONENT_H_

#include "Component.h"

#include "OgreSceneNode.h"

namespace Thermite
{
	class RenderComponent : public Component
	{
	public:
		RenderComponent(Object* parent);
		~RenderComponent(void);

		void update(void);

	public:

		Ogre::SceneNode* mOgreSceneNode;
	};
}

#endif //RENDER_COMPONENT_H_
