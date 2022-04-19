#include <iostream>
#include <cstdlib>

#include "stack.h"



// Initialising stack
Stack::Stack(int size) {
	arr = new uint16_t[size];
	capacity = size;
	top = -1;
}

// Destructor to free allocated memory
Stack::~Stack() {
	delete[] arr;
}

// Push function to add element 'x' to the stack
void Stack::push(uint16_t x) {
	// Returns an error if the stack is full and terminates
	if (isFull()) {
		std::cout << "Overflow Error. Terminated\n";
		exit(EXIT_FAILURE);
	}

	// Increments the top index by 1 and sets that 
	// value in arr to x
	arr[++top] = x;
}

// Pop function to remove the top element from the stack
void Stack::pop() {
	// Checks if there are values in the array before 
	// decrementing the top index, making anything above inaccessible 
	if(!isEmpty())
		top--;
	else
		exit(EXIT_FAILURE);
}

// Peek function to return the top element of teh stack
uint16_t Stack::peek() {
	// Makes sure there is a value to return before returning
	if (!isEmpty()) 
		return arr[top];
	else
		exit(EXIT_FAILURE);
}

// Size function to simply return the size of the stack
int Stack::size() {
	return top + 1;
}

// Uses the top attribute to check if the stack is empty
bool Stack::isEmpty() {
	return top  == -1;
}

// Uses the top attribute and the capacity
// attribute to check if the stack is full
bool Stack::isFull() {
	return top == capacity - 1;
}
