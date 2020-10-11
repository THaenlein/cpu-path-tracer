/*
 * SynchronizedQueue.hpp
 */

#pragma once

/*--------------------------------< Includes >-------------------------------------------*/
#include <queue>
#include <mutex>

#include "exceptions.hpp"

namespace raytracing
{
	/*--------------------------------< Defines >-------------------------------------------*/

	/*--------------------------------< Typedefs >------------------------------------------*/

	using lock = std::lock_guard<std::mutex>;
	using uniqueLock = std::unique_lock<std::mutex>;

	/*--------------------------------< Constants >-----------------------------------------*/

	template <typename T>
	class SynchronizedQueue
	{
	/*--------------------------------< Public methods >------------------------------------*/
	public:

		void pushBack(T const& value)
		{
			lock l(this->mutex); // prevents multiple pushes corrupting queue_
			bool wake = this->queue.empty(); // we may need to wake consumer
			this->queue.push(value);
			if (wake)
			{
				this->conditionVariable.notify_one();
			}
		}


		T popFront()
		{
			uniqueLock u(mutex);
			while (this->queue.empty())
			{
				this->conditionVariable.wait(u);
			}
			// now queue_ is non-empty and we still have the lock
			T returnValue = this->queue.front();
			this->queue.pop();
			return returnValue;
		}


		bool popFront(T& queueEntry)
		{
			uniqueLock u(mutex);
			if (this->queue.empty())
			{
				return false;
			}
			// now queue_ is non-empty and we still have the lock
			queueEntry = this->queue.front();
			this->queue.pop();
			return true;
		}
	
	/*--------------------------------< Protected methods >---------------------------------*/
	protected:
	
	/*--------------------------------< Private methods >-----------------------------------*/
	private:
	
	/*--------------------------------< Public members >------------------------------------*/
	public:
	
	/*--------------------------------< Protected members >---------------------------------*/
	protected:
	
	/*--------------------------------< Private members >-----------------------------------*/
	private:

		std::queue<T> queue;

		std::mutex mutex;

		std::condition_variable conditionVariable;

	};
	
} // end of namespace raytracing