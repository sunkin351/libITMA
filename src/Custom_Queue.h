/*
	This file is a part of the ITMA (Inter-Thread Messaging Achitecture) project under the MPLv2.0 or later license.
	See file "LICENSE" or https://www.mozilla.org/en-US/MPL/2.0/ for details.

	Please do take note, this is a specialized container specifically for this library.
	I cannot garentee it to work with everything like std::vector.

	It is recommended that if you do choose to use this class outside the library that you use
	objects that have std::move support.
*/

#pragma once

#include <memory>
#include <deque>

namespace ITMA
{
	template<class T>
	class CustomQueue
	{
		std::deque<T> queue;
	public:
		CustomQueue(){}

		CustomQueue(CustomQueue<T> && src)
		{
			*this = std::move(src);
		}

		~CustomQueue()
		{
			clear();
		}

		inline bool empty() { return (size() == 0); }
		size_t size() { return queue.size(); }

		void push(T& src)
		{
			queue.push_back(src);
		}

		void push(T&& src)
		{
			queue.emplace_back(std::move(src));
		}

		void push_noCopy(T& src)
		{
			push(std::move(src));
		}

		void pop(T& dest)
		{
			dest = queue.front();
			queue.pop_front();
		}

		void clear()
		{
			queue.clear();
		}

		void operator=(CustomQueue<T> && src)
		{
			clear();
			queue = std::move(src.queue);
		}
	private:
		CustomQueue<T>& operator=(CustomQueue<T> & src) = delete;
		CustomQueue(CustomQueue<T> & src) = delete;
	};

}