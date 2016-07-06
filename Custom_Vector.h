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

namespace ITMA
{
	template<class T>
	void move(T * Dest, T * Src, int DestStartingElement, int SrcStartingElement, int count)
	{
		register int dsp = DestStartingElement;
		register int ssp = SrcStartingElement;
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
		int size = 0;
		int ArraySize = 0;
	public:
		CustomVector() {}

		~CustomVector() { clear(); }

		void push_back(T& obj);
		void push_back(T&& src);
		void pop_back();
		void remove(unsigned int elem);
		T & operator[](unsigned int ElemNum);

		int size() { return size; } //Returns number of objects in storage
		bool is_empty() { return (size == 0); };
		int Max() { return ArraySize; } //Returns max currently allocated

		void clear()
		{
			delete[] array;
			array = 0;
			size = 0;
			ArraySize = 0;
		}

		void reserve(int size);
		void shrink_to_fit();

	private:
		void realloc(int size);
		T* allocate(int size);

		//It is the responsibility of the programmer to ensure that there is enough space in the destination array, 
		//and that this function will not iterate over the edge of the source array.
	};

	template<class T, int ObjectSize>
	void CustomVector<T, ObjectSize>::push_back(T& obj)
	{
		if (size == ArraySize)
		{
			realloc(ArraySize + (5 - (ArraySize % 5)));
		}

		array[size] = obj;
		++size;
	}

	template<class T, int ObjectSize>
	void CustomVector<T, ObjectSize>::push_back(T&& src)
	{
		if (size == ArraySize)
		{
			realloc(ArraySize + (5 - (ArraySize % 5)));
		}

		array[size] = std::move(src);
		++size;
	}

	template<class T, int ObjectSize>
	void CustomVector<T, ObjectSize>::pop_back()
	{
		if (size == 0)
		{
			throw std::exception("No Elements exception");
		}
		--size;
		array[size] = T();
	}

	template<class T, int ObjectSize>
	void CustomVector<T, ObjectSize>::remove(unsigned int elem)
	{
		if (elem >= size)
		{
			throw std::exception("Out of Bounds exception");
		}

		T* bufptr = allocate(ArraySize);

		if (elem != size - 1 && elem != 0)
		{
			register int temp1 = elem + 1;

			move(bufptr, array, 0, 0, elem);
			move(bufptr, array, elem, temp1, size - temp1);

			delete[] array;
			array = bufptr;
		}
		else if (elem == 0)
		{

		}
	}

	template<class T, int ObjectSize>
	T& CustomVector<T, ObjectSize>::operator[](unsigned int ElemNum)
	{
		if (ElemNum >= size)
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
	void CustomVector<T, ObjectSize>::reserve(int size)
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
		realloc(size);
	}

	template<class T, int ObjectSize>
	void CustomVector<T, ObjectSize>::realloc(int size)
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

	template<class T, int ObjectSize>
	inline T * CustomVector<T, ObjectSize>::allocate(int size)
	{
		return new T[size];
	}
}
