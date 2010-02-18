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

#ifndef __THERMITE_THREADSAFEQUEUE_H__
#define __THERMITE_THREADSAFEQUEUE_H__

#include <QMutex>

#include <queue>

namespace Thermite
{
	template<typename Type>
	class ThreadSafeQueue
	{
	public:
		ThreadSafeQueue(void)
		{
		}

		~ThreadSafeQueue(void)
		{			
		}

		bool empty(void) const
		{
			m_mutex.lock();
			bool result = m_queue.empty();
			m_mutex.unlock();
			return result;
		}

		size_t size(void) const
		{
			m_mutex.lock();
			size_t result = m_queue.size();
			m_mutex.unlock();
			return result;
		}

		void push(const Type& value)
		{
			m_mutex.lock();
			m_queue.push(value);
			m_mutex.unlock();
		}

		void pop(void)
		{
			m_mutex.lock();
			m_queue.pop();
			m_mutex.unlock();
		}

		Type& front(void)
		{
			m_mutex.lock();
			Type& result = m_queue.front();
			m_mutex.unlock();
			return result;
		}

		const Type& front(void) const
		{
			m_mutex.lock();
			Type& result = m_queue.front();
			m_mutex.unlock();
			return result;
		}

		Type& back(void)
		{
			m_mutex.lock();
			Type& result = m_queue.back();
			m_mutex.unlock();
			return result;
		}

		const Type& back(void) const
		{
			m_mutex.lock();
			Type& result = m_queue.back();
			m_mutex.unlock();
			return result;
		}

	private:
		mutable QMutex m_mutex;
		std::queue<Type> m_queue;
	};
}

#endif //__THERMITE_THREADSAFEQUEUE_H__