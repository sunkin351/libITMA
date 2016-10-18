#pragma once
/*	
	This file is a part of the ITMA (Inter-Thread Messaging Achitecture) project under the MPLv2.0 or later license.
	See file "LICENSE" or https://www.mozilla.org/en-US/MPL/2.0/ for details.

	Please do take note, this is a specialized container specifically for this library. 
	I cannot garentee it to work with everything like std::vector.

	It is recommended that if you do choose to use this class outside the library that you use
	objects that have std::move support.
*/

#include <exception>
#include <memory>
#include <cstdint>

namespace ITMA
{
	template<class T> //Warning: Do not use if you don't know what you are doing.
	void move(T * Dest, T * Src, size_t DestStartingElement, size_t SrcStartingElement, size_t count) 
	{
		uint64_t dsp = DestStartingElement;
		uint64_t ssp = SrcStartingElement;
		for (int i = 0; i != count; ++i)
		{
			Dest[dsp] = std::move(Src[ssp]);
			++dsp;
			++ssp;
		}
	}

	template<class T, int ObjectSize = sizeof(T)>
	class CustomVector
	{
		T* array = 0;
		size_t objects = 0;
		size_t ArraySize = 0;
	public:
		CustomVector() {}

		~CustomVector() { clear(); }

		void push_back(T& obj);
		void push_back(T&& src);
		void pop_back();
		void remove(size_t elem);
		T & operator[](size_t ElemNum);

		size_t size() { return objects; } //Returns number of objects in storage
		bool is_empty() { return (objects == 0); };
		size_t Max() { return ArraySize; } //Returns max currently allocated

		void clear()
		{
			delete[] array;
			array = 0;
			objects = 0;
			ArraySize = 0;
		}

		void reserve(size_t size);
		void shrink_to_fit();

	private:
		void realloc(size_t size);
		inline T* allocate(size_t size)
		{
			return new T[size];
		}

		//It is the responsibility of the programmer to ensure that there is enough space in the destination array, 
		//and that this function will not iterate over the edge of the source array.
	};

	template<class T, int ObjectSize>
	void CustomVector<T, ObjectSize>::push_back(T& obj)
	{
		if (objects == ArraySize)
		{
			realloc(ArraySize + (5 - (ArraySize % 5)));
		}

		array[objects] = obj;
		++objects;
	}

	template<class T, int ObjectSize>
	void CustomVector<T, ObjectSize>::push_back(T&& src)
	{
		if (objects == ArraySize)
		{
			realloc(ArraySize + (5 - (ArraySize % 5)));
		}

		array[objects] = std::move(src);
		++objects;
	}

	template<class T, int ObjectSize>
	void CustomVector<T, ObjectSize>::pop_back()
	{
		if (objects == 0)
		{
			throw std::exception("No Elements exception");
		}
		--objects;
		array[objects] = T();
	}

	template<class T, int ObjectSize>
	void CustomVector<T, ObjectSize>::remove(size_t elem)
	{
		if (elem >= objects)
		{
			throw std::exception("Out of Bounds exception");
		}

		T* bufptr = allocate(ArraySize);

		if (elem != objects - 1 && elem != 0)
		{
			register size_t temp1 = elem + 1;

			move(bufptr, array, 0, 0, elem);
			move(bufptr, array, elem, temp1, objects - temp1);

			delete[] array;
			array = bufptr;
		}
		else if (elem == 0)
		{

		}
	}

	template<class T, int ObjectSize>
	T& CustomVector<T, ObjectSize>::operator[](size_t ElemNum)
	{
		if (ElemNum >= objects)
		{
			throw std::exception("Out of Bounds Exception");
		}

		if (ElemNum == 0)
		{
			return *array;
		}

		auto obj = &array[0];
		obj += ElemNum;
		return *obj;
	}

	template<class T, int ObjectSize>
	void CustomVector<T, ObjectSize>::reserve(size_t size)
	{
		register int temp = ArraySize - size;
		if (size > temp)
		{
			realloc(ArraySize + size);
		}
	}

	template<class T, int ObjectSize>
	void CustomVector<T, ObjectSize>::shrink_to_fit()
	{
		realloc(objects);
	}

	template<class T, int ObjectSize>
	void CustomVector<T, ObjectSize>::realloc(size_t size)
	{
		if (!array)
		{
			array = allocate(size);
		}
		else {
			T* buf = new T[size];
			if (size < ArraySize)
			{
				move(buf, array, 0, 0, size);
			}
			else {
				move(buf, array, 0, 0, ArraySize);
			}
			delete[] array;
			array = buf;
		}
		ArraySize = size;
	}
}
