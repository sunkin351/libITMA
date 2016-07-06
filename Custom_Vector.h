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
	void move(T * dest, T * src, int dest_start, int src_start, int count)
	{
		register int dsp = dest_start;
		register int ssp = src_start;
		for (int i = 0; i != count; ++i)
		{
			dest[dsp] = std::move(src[ssp]);
			++dsp;
			++ssp;
		}
	}

	template<class T, int objsize = sizeof(T)>
	class custom_vector
	{
		T* array = 0;
		int ObjNum = 0;
		int CurrMax = 0;
	public:
		custom_vector() {}

		~custom_vector() { clear(); }

		void push_back(T& obj);
		void push_back(T&& src);
		void pop_back();
		void remove(unsigned int elem);
		T & operator[](unsigned int ElemNum);

		int size() { return ObjNum; } //Returns number of objects in storage
		bool is_empty() { return (ObjNum == 0); };
		int Max() { return CurrMax; } //Returns max currently allocated

		void clear()
		{
			delete[] array;
			array = 0;
			ObjNum = 0;
			CurrMax = 0;
		}

		void reserve(int size);
		void shrink_to_fit();

	private:
		void realloc(int size);
		T* allocate(int size);

		//It is the responsibility of the programmer to ensure that there is enough space in the destination array, 
		//and that this function will not iterate over the edge of the source array.
	};

	template<class T, int objsize = sizeof(T)>
	void custom_vector<T, objsize>::push_back(T& obj)
	{
		if (ObjNum == CurrMax)
		{
			realloc(CurrMax + (5 - (CurrMax % 5)));
		}

		array[ObjNum] = obj;
		++ObjNum;
	}

	template<class T, int objsize = sizeof(T)>
	void custom_vector<T, objsize>::push_back(T&& src)
	{
		if (ObjNum == CurrMax)
		{
			realloc(CurrMax + (5 - (CurrMax % 5)));
		}

		array[ObjNum] = std::move(src);
		++ObjNum;
	}

	template<class T, int objsize = sizeof(T)>
	void custom_vector<T, objsize>::pop_back()
	{
		if (ObjNum == 0)
		{
			throw std::exception("No Elements exception");
		}
		--ObjNum;
		array[ObjNum] = T();
	}

	template<class T, int objsize = sizeof(T)>
	void custom_vector<T, objsize>::remove(unsigned int elem)
	{
		if (elem >= ObjNum)
		{
			throw std::exception("Out of Bounds exception");
		}

		T* bufptr = allocate(CurrMax);

		if (elem != ObjNum - 1 && elem != 0)
		{
			register int temp1 = elem + 1;

			move(bufptr, array, 0, 0, elem);
			move(bufptr, array, elem, temp1, ObjNum - temp1);

			delete[] array;
			array = bufptr;
		}
		else if (elem == 0)
		{

		}
	}

	template<class T, int objsize = sizeof(T)>
	T& custom_vector<T, objsize>::operator[](unsigned int ElemNum)
	{
		if (ElemNum >= ObjNum)
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

	template<class T, int objsize = sizeof(T)>
	void custom_vector<T, objsize>::reserve(int size)
	{
		register int temp = CurrMax - ObjNum;
		if (size > temp)
		{
			realloc(CurrMax + size);
		}
	}

	template<class T, int objsize = sizeof(T)>
	void custom_vector<T, objsize>::shrink_to_fit()
	{
		realloc(ObjNum);
	}

	template<class T, int objsize = sizeof(T)>
	void custom_vector<T, objsize>::realloc(int size)
	{
		if (!array)
		{
			array = allocate(size);
		}
		else {
			T* buf = new T[size];
			if (size < CurrMax)
			{
				move(buf, array, 0, 0, size);
			}
			else {
				move(buf, array, 0, 0, CurrMax);
			}
			delete[] array;
			array = buf;
		}
		CurrMax = size;
	}

	template<class T, int objsize>
	inline T * custom_vector<T, objsize>::allocate(int size)
	{
		return new T[size];
	}
}
