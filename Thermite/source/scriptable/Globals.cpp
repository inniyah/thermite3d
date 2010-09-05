#include "Globals.h"

namespace Thermite
{
	//Our single globals object
	Globals globals;

	Globals::Globals(QObject* parent)
		:QObject(parent)
	{
		mTimeSinceAppStart.start();
	}

	int Globals::timeSinceAppStart(void) const
	{
		return mTimeSinceAppStart.elapsed();
	}

	/*quint32 Globals::getCurrentFrameTime(void) const
	{
		return m_uCurrentFrameTime;
	}

	void Globals::setCurrentFrameTime(const quint32 uCurrentFrameTime)
	{
		m_uCurrentFrameTime = uCurrentFrameTime;
	}

	quint32 Globals::getPreviousFrameTime(void) const
	{
		return m_uPreviousFrameTime;
	}

	void Globals::setPreviousFrameTime(const quint32 uPreviousFrameTime)
	{
		m_uPreviousFrameTime = uPreviousFrameTime;
	}*/
}
