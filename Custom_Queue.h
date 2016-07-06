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

namespace ITMA
{

	template<class T>
	struct queue_object
	{
		queue_object<T> * nextObj = nullptr;
		T _object;
	};

	template<class T, class obj = queue_object<T>>
	class custom_queue
	{
		obj* FirstElement;
		obj* LastElement;
		int queueSize = 0;
	public:
		custom_queue()
		{
			FirstElement = nullptr;
			LastElement = nullptr;
		}

		custom_queue(custom_queue<T> && src)
		{
			*this = std::move(src);
		}

		~custom_queue()
		{
			clear();
		}

		bool is_empty() { return (queueSize == 0); }
		int size() { return queueSize; }

		void push(T& src)
		{
			obj* bufptr = new obj;
			bufptr->_object = src;

			if (FirstElement == nullptr && LastElement == nullptr)
			{
				FirstElement = bufptr;
				LastElement = bufptr;
			}
			else {
				LastElement->nextObj = bufptr;
				LastElement = bufptr;
			}
			++queueSize;
		}

		void push(T&& src)
		{
			obj* bufptr = new obj;
			bufptr->_object = std::move(src);

			if (FirstElement == nullptr && LastElement == nullptr)
			{
				FirstElement = bufptr;
				LastElement = bufptr;
			}
			else {
				LastElement->nextObj = bufptr;
				LastElement = bufptr;
			}
			++queueSize;
		}

		void push_noCopy(T& src)
		{
			push(std::move(src));
		}

		void pop(T& dest)
		{
			if (FirstElement == nullptr)
			{
				throw std::exception("NullPointer Exception");
			}

			dest = FirstElement->_object;
			if (FirstElement->nextObj != nullptr)
			{
				obj* bufptr = FirstElement->nextObj;
				delete FirstElement;
				FirstElement = bufptr;
			}
			else if (FirstElement == LastElement)
			{
				delete FirstElement;
				FirstElement = nullptr;
				LastElement = nullptr;
			}
			--queueSize;
		}

		void clear()
		{
			if (FirstElement == nullptr)
			{
				return;
			}

			while (FirstElement != LastElement)
			{
				obj* bufptr = FirstElement->nextObj;
				delete FirstElement;
				FirstElement = bufptr;
			}

			delete FirstElement;
			FirstElement = nullptr;
			LastElement = nullptr;

			queueSize = 0;
		}

		void operator=(custom_queue<T> && src)
		{
			clear();

			FirstElement = src.FirstElement;
			LastElement = src.LastElement;
			queueSize = src.queueSize;

			src.FirstElement = 0;
			src.LastElement = 0;
			src.queueSize = 0;
		}
	private:
		custom_queue<T>& operator=(custom_queue<T> & src) = delete;
		custom_queue(custom_queue<T> & src) = delete;
	};

}