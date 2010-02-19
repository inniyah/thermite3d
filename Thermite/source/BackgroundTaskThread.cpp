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

#include "BackgroundTaskThread.h"

#include <QMutex>
#include <QRunnable>
#include <QSemaphore>

#include <algorithm>

namespace Thermite
{
	BackgroundTaskThread::BackgroundTaskThread(QObject* parent)
		:QThread(parent)
	{
		m_runnableContainerMutex = new QMutex;
		m_noOfRunnables = new QSemaphore;
	}

	BackgroundTaskThread::~BackgroundTaskThread(void)
	{
		delete m_runnableContainerMutex;
		delete m_noOfRunnables;
	}

	void BackgroundTaskThread::run(void)
	{
		while(true)
		{
			msleep(100);

			m_noOfRunnables->acquire();

			m_runnableContainerMutex->lock();
			QRunnable* runnable = m_runnableContainer.front();
			m_runnableContainer.pop_front();
			m_runnableContainerMutex->unlock();

			runnable->run();

			if(runnable->autoDelete())
			{
				delete runnable;
			}
		}
	}

	void BackgroundTaskThread::addRunnable(QRunnable* runnable)
	{
		m_runnableContainerMutex->lock();
		m_runnableContainer.push_back(runnable);
		m_noOfRunnables->release();
		m_runnableContainerMutex->unlock();
	}

	bool BackgroundTaskThread::removeRunnable(QRunnable* runnable)
	{
		bool bRemoved = false;

		m_runnableContainerMutex->lock();

		std::list<QRunnable*>::iterator iterRunnable = std::find(m_runnableContainer.begin(), m_runnableContainer.end(), runnable);
		if(iterRunnable != m_runnableContainer.end())
		{
			m_runnableContainer.erase(iterRunnable);
			m_noOfRunnables->acquire();
			bRemoved = true;
		}

		m_runnableContainerMutex->unlock();

		return bRemoved;
	}
}