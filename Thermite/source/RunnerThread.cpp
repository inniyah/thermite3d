#pragma region License
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
#pragma endregion

#include "RunnerThread.h"

#include <QMutex>
#include <QRunnable>
#include <QSemaphore>

namespace Thermite
{
	RunnerThread::RunnerThread(QObject* parent)
		:QThread(parent)
	{
		m_runnableQueueMutex = new QMutex;
		m_noOfRunnables = new QSemaphore;
	}

	RunnerThread::~RunnerThread(void)
	{
		delete m_runnableQueueMutex;
		delete m_noOfRunnables;
	}

	void RunnerThread::run(void)
	{
		while(true)
		{
			m_noOfRunnables->acquire();

			m_runnableQueueMutex->lock();
			QRunnable* runnable = m_runnableQueue.front();
			m_runnableQueue.pop();
			m_runnableQueueMutex->unlock();

			runnable->run();

			if(runnable->autoDelete())
			{
				delete runnable;
			}
		}
	}

	void RunnerThread::startRunnable(QRunnable* runnable, int /*priority*/)
	{
		m_runnableQueueMutex->lock();
		m_runnableQueue.push(runnable);
		m_runnableQueueMutex->unlock();

		m_noOfRunnables->release();
	}
}