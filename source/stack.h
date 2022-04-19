// stack class
#ifndef STACK_H // include guard
#define STACK_H

#define SIZE 0x10

#include <stdint.h>

class Stack {
	// Private variables that can only be accessed by the methods of the class
	private:
		uint16_t *arr;
		int top;
		int capacity;
	// Public methods that will access and change the private variables
	public:
		Stack(int size = SIZE);		// constructor
		~Stack(); 			// destructor

		// These are essentially the getters and the setters
		void push(uint16_t);
		void pop();
		uint16_t peek();

		// These functions will get information about the stack
		int size();
		bool isEmpty();
		bool isFull();
};


#endif //STACK_H
