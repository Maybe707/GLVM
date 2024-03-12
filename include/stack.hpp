// This file is part of Game Loop Versatile Modules (GLVM)
// Copyright © 2024 Maksim Manokhin a.k.a. Yuriorkis_Scream. Contacts: <fellfrostqtw@gmail.com>
// Author: Maksim Manokhin a.k.a. Yuriorkis_Scream
// License: http://opensource.org/licenses/MIT

#ifndef CLASSIC_STACK
#define CLASSIC_STACK

#include <assert.h>

namespace GLVM::core
{    

	template <class T>
	class stack
	{
		unsigned int size_ = 0;
		unsigned int capacity = 0;
		unsigned int expander = 10;
	
		T* data = nullptr;
	public:
		stack() {
			data = new T[expander];
		}

		stack(const stack& stack) {
			this->size_ = stack.size_;
			this->capacity = stack.capacity;
			this->expander = stack.expander;
			T* temp = new T[stack.capacity];
			for ( unsigned int i = 0; i < stack.size_; ++ i )
				temp[i] = stack.data[i];

			data = temp;
		}
		
		void push(T element) {
			if (size_ == capacity) {
				unsigned int oldCapacity = capacity;
				capacity += capacity / 2 + expander;
				T* temp = new T[capacity];
				for ( unsigned int i = 0; i < oldCapacity; ++i )
					temp[i] = data[i];

				delete [] data;
				data = temp;
			}

			data[size_] = element;
			++size_;
		}
	
		T pop() {
			assert(size_ > 0);
			
			--size_;
			data[size_] = 999;
			return data[size_];
		}

		T& top() {
			assert(size_ > 0);
			unsigned int top = size_ - 1;
			return data[top];
		}

		bool contains(T element) {
			for ( unsigned int i = 0; i < size_; ++i ) {
				if ( data[i] == element )
					return true;
			}

			return false;
		}

		unsigned int size() { return size_; }
		bool empty() { return size_ == 0; }
		T operator[](unsigned int index) { return data[index]; }

		void remove(T element) {
			bool flag = false;
			for ( unsigned int i = 0; i < size_; ++i ) {
				if ( data[i] == element ) 
					flag = true;

				if ( flag && i == size_ - 1 )
					data[i] = 999;
				
				if ( flag ) 
					data[i] = data[i + 1];
			}

			if ( flag )
				size_ -= 1;
		}

		~stack() {
			delete [] data;
		}
	};

} // namespace GLVM::core
	
#endif

