/*
	This file is a part of the ITMA (Inter-Thread Messaging Achitecture) project under the MPLv2.0 or later license.
	See file "LICENSE" or https://www.mozilla.org/en-US/MPL/2.0/ for details.

	Please do take note, this is a specialized container specifically for this library.
	I cannot gaurentee it to work with everything like std::vector.

	It is recommended that if you do choose to use this class outside the library that you use
	objects that have std::move support.
*/

#pragma once
#ifndef SRC_CUSTOM_QUEUE_H
#define SRC_CUSTOM_QUEUE_H

#include <memory>
#include <deque>

#include "unknown_exception.h"

#define Check(x) if(x == 0) return;

namespace ITMA
{
	template<class T>
	class QueueObject{
		QueueObject<T>* next = 0;
		T object;
	};

	template<class T>
	class CustomQueue
	{
		typedef QueueObject<T> Object;
		typedef CustomQueue<T> Queue;

		Object* First = 0;
		Object* Last = 0;
		long queue_size = 0;
		std::deque<T> queue;
	public:
		CustomQueue(){}

		CustomQueue(Queue && src)
		{
			*this = std::move(src);
		}

		~CustomQueue()
		{
			clear();
		}

		bool empty() { return (queue_size == 0); }
		size_t size() { return queue_size; }

		void push(T& src)
		{
			push(std::move(src));
		}

		void push(T&& src)
		{
			Object* temp = new Object();
			temp->object = std::move(src);

			if(First == nullptr)
			{
				First = Last = temp;
			}
			else{
				Last->next = temp;
				Last = temp;
			}
			++queue_size;
		}

		void push_noCopy(T& src)
		{
			push(std::move(src));
		}

		void pop(T& dest)
		{
			check();
			Check(First)

			dest = std::move(First->object);

			pop();
		}

		T peek()
		{
			return First->object;
		}

		void clear()
		{
			check();
			Check(First)

			while(First != nullptr)
			{
				pop();
			}
		}

		void operator=(Queue && src)
		{
			clear();
			First = src.First;
			Last = src.Last;
			queue_size = src.queue_size;

			src.First = nullptr;
			src.Last = nullptr;
			src.queue_size = 0;
		}
	private:
		void check()
		{
			if(First == nullptr && queue_size != 0)
			{
				throw UnknownException();
			}
		}

		void pop()
		{
			if(First->next == nullptr)
			{
				delete First;
				First = nullptr;
			}
			else{
				Object* tmp = First->next;
				delete First;
				First = tmp;
			}
		}

		Queue& operator=(Queue<T> & src) = delete;
		CustomQueue(Queue<T> & src) = delete;
	};

}

#undef Check
#endif
