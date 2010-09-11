#include "Keyboard.h"

namespace Thermite
{
	Keyboard::Keyboard(QObject * parent)
		:QObject(parent)
	{
	}

	bool Keyboard::isPressed(int key)
	{
		return mKeyStates[key];
	}

	void Keyboard::press(int key)
	{
		mKeyStates[key] = true;
	}

	void Keyboard::release(int key)
	{
		mKeyStates[key] = false;
	}
}
