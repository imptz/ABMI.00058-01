#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include "iterator.h"
#include "memory.h"

template<class T>
class DynamicArray : public IIterator<T>{
public:
	typedef Iterator<T> iterator;

private:
	friend class Iterator<T>;

	int position;

	virtual T* getNextPointer(T* _pItem){
		if (position < size - 1){
			position++;
			return ++_pItem;
		}
		else
			return nullptr;
	}

public:
	virtual Iterator<T> begin(){
		position = 0;
		return Iterator<T>(this, pData);
	}

protected:
	T* pData;
	int size;

public:
	DynamicArray()
		:	position(0), pData(nullptr), size(0)
	{}

	virtual ~DynamicArray(){
		if (pData != nullptr)
			delete[] pData;
	}

	T& operator[](int index){
		return pData[index];
	}

	void resize(int newSize){
		size = newSize;
		T* newDataPointer = new T[newSize];
		if (pData != nullptr)
		{
			memcpy(newDataPointer, pData, newSize * sizeof(T));
			delete[] pData;
		}

		pData = newDataPointer;
	}

	int getSize(){
		return size;
	}

	void swap(int indexL, int indexR){
		T temp = pData[indexL];
		pData[indexL] = pData[indexR];
		pData[indexR] = temp;
	}
};

template<class T>
class SortedDynamicArray : public DynamicArray<T>{
public:
	template<typename D>
	void orderBy(D T::* field, bool descending = false){
		if (field == nullptr || this->pData == nullptr || this->size == 0)
			return;

		int jump = this->size;
		bool swapped = true;

		while (jump > 1 || swapped){
			if (jump > 1)
				jump = static_cast<int>(jump / 1.25);

			swapped = false;

			for (int i = 0; i + jump < this->size; ++i)
				if (!descending){
					if (this->pData[i].*field > this->pData[i + jump].*field){
						this->swap(i, i + jump);
						swapped = true;
					}
				}
				else{
					if (this->pData[i].*field < this->pData[i + jump].*field){
						this->swap(i, i + jump);
						swapped = true;
					}
				}
		}
	}

	template<typename D, typename E>
	void orderBy(D T::* field1, E T::* field2, bool descending1 = false, bool descending2 = false){
		orderBy(field1, descending1);

		if (field2 == nullptr || this->pData == nullptr || this->size == 0)
			return;

		int jump = this->size;
		bool swapped = true;

		while (jump > 1 || swapped){
			if (jump > 1)
				jump = static_cast<int>(jump / 1.25);

			swapped = false;

			for (int i = 0; i + jump < this->size; ++i){
				bool res1 = (this->pData[i].*field1 == this->pData[i + jump].*field1);
				bool res2 = (this->pData[i].*field2 > this->pData[i + jump].*field2);

				if (descending2)
					res2 = !res2;

				if (res1 && res2){
					this->swap(i, i + jump);
					swapped = true;
				}
			}
		}
	}

	template<typename D>
	D getMax(D T::* field){
		if (field == nullptr || this->pData == nullptr || this->size == 0)
			return D();

		D res;
		bool first = true;

		for (int i = 0; i < this->size; ++i){
			if (first){
				first = false;
				res = this->pData[i].*field;
			}
			else{
				if (res < this->pData[i].*field)
					res = this->pData[i].*field;
			}
		}

		return res;
	}

	template<typename D>
	D getMin(D T::* field){
		if (field == nullptr || this->pData == nullptr || this->size == 0)
			return D();

		D res;
		bool first = true;

		for (int i = 0; i < this->size; ++i){
			if (first){
				first = false;
				res = this->pData[i].*field;
			}
			else{
				if (res > this->pData[i].*field)
					res = this->pData[i].*field;
			}
		}

		return res;
	}
};
#endif
