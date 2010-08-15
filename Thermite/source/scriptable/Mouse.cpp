#include "Mouse.h"

Mouse::Mouse(QObject * parent)
	:QObject(parent)
	,mWheelDelta(0)
{
}

bool Mouse::isPressed(int mouseButton)
{
	//Note - I'd rather pass a Qt::MouseButton in a parameter to this 
	//function and avoid the class, but I had problems registering it
	//with qScriptRegisterMetaType().
	Qt::MouseButton mb = static_cast<Qt::MouseButton>(mouseButton);

	return mMouseButtons & mb;
}

void Mouse::press(int mouseButton)
{
	//Note - I'd rather pass a Qt::MouseButton in a parameter to this 
	//function and avoid the class, but I had problems registering it
	//with qScriptRegisterMetaType().
	Qt::MouseButton mb = static_cast<Qt::MouseButton>(mouseButton);

	mMouseButtons |= mb;
}

void Mouse::release(int mouseButton)
{
	//Note - I'd rather pass a Qt::MouseButton in a parameter to this 
	//function and avoid the class, but I had problems registering it
	//with qScriptRegisterMetaType().
	Qt::MouseButton mb = static_cast<Qt::MouseButton>(mouseButton);

	mMouseButtons &= ~mb;
}

const QPoint& Mouse::position(void)
{
	return mPosition;
}

void Mouse::setPosition(const QPoint& pos)
{
	mPosition = pos;
}

const QPoint& Mouse::previousPosition(void)
{
	return mPreviousPosition;
}

void Mouse::setPreviousPosition(const QPoint& pos)
{
	mPreviousPosition = pos;
}

void Mouse::modifyWheelDelta(int wheelDelta)
{
	mWheelDelta += wheelDelta;
}

int Mouse::getWheelDelta(void)
{
	return mWheelDelta;
}

void Mouse::resetWheelDelta(void)
{
	mWheelDelta = 0;
}