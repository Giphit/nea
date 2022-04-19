#ifndef CHIP_8_H
#define CHIP_8_H

#include <stdint.h>

#include "stack.h"

class Chip8 {
	private:
		Stack stack;				// Call Stack

		uint8_t memory[4096];			// 4KB Memory
		uint8_t V[16];				// Registers V0 to VF

		uint16_t pc;				// Program Counter
		uint16_t opcode;			// Current Op Code
		uint16_t I;				// Index Register

		uint8_t delayTimer;			// Delay Timer
		uint8_t soundTimer;			// Sound Timer
		void init();				// Initialise the emulator

	public:
		Chip8();				// Constructor
		~Chip8();				// Destructor

		uint8_t graphics[2048];			// Graphical buffer
		uint8_t key[16];			// Keypad

		bool drawFlag;				// Flag to indicate a draw has occured

		void fdeCycle();			// Emulate One CPU Cycle

		bool loadRom(const char *filePath);	// Load Application
};
#endif // CHIP_8_H
