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
	struct QueueObject
	{
		QueueObject<T> * nextObj = nullptr;
		T _object;
	};

	template<class T, class obj = QueueObject<T>>
	class CustomQueue
	{
		obj* FirstElement;
		obj* LastElement;
		int size = 0;
	public:
		CustomQueue()
		{
			FirstElement = nullptr;
			LastElement = nullptr;
		}

		CustomQueue(CustomQueue<T> && src)
		{
			*this = std::move(src);
		}

		~CustomQueue()
		{
			clear();
		}

		bool is_empty() { return (size == 0); }
		int size() { return size; }

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
			++size;
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
			++size;
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
			--size;
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

			size = 0;
		}

		void operator=(CustomQueue<T> && src)
		{
			clear();

			FirstElement = src.FirstElement;
			LastElement = src.LastElement;
			size = src.size;

			src.FirstElement = 0;
			src.LastElement = 0;
			src.size = 0;
		}
	private:
		CustomQueue<T>& operator=(CustomQueue<T> & src) = delete;
		CustomQueue(CustomQueue<T> & src) = delete;
	};

}