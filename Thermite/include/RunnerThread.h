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

#ifndef __THERMITE_RUNNER_THREAD_H__
#define __THERMITE_RUNNER_THREAD_H__

#include <QThread>

#include <list>

class QMutex;
class QRunnable;
class QSemaphore;

namespace Thermite
{
	class RunnerThread : public QThread
	{
	public:
		RunnerThread(QObject* parent=0);
		~RunnerThread(void);

		void run(void);

		void addRunnable(QRunnable* runnable);
		bool removeRunnable(QRunnable* runnable);

	protected:
		std::list<QRunnable*> m_runnableContainer;

		QSemaphore* m_noOfRunnables;
		QMutex* m_runnableContainerMutex;
	};
}

#endif //__THERMITE_RUNNER_THREAD_H__