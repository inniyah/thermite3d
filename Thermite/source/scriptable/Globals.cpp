#include "Globals.h"

Globals::Globals(QObject* parent)
	:QObject(parent)
{
}

quint32 Globals::getCurrentFrameTime(void) const
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
}