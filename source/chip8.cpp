#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <random>
#include <time.h>
#include <fstream>

#include "chip8.h"

// Font set for CHIP8 
unsigned char fontSet[80] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, //0
	0x20, 0x60, 0x20, 0x20, 0x70, //1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
	0x90, 0x90, 0xF0, 0x10, 0x10, //4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
	0xF0, 0x10, 0x20, 0x40, 0x40, //7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
	0xF0, 0x90, 0xF0, 0x90, 0x90, //A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
	0xF0, 0x80, 0x80, 0x80, 0xF0, //C
	0xE0, 0x90, 0x90, 0x90, 0xE0, //D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
	0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};

Chip8::Chip8() {}
Chip8::~Chip8() {}

/* These functions are empty because
   the Chip8 class will be entirely 
   deconstructed once the program is terminated anyway
   and the construction is handled by init() */

// Initialise
void Chip8::init() {
	pc = 0x200;	// Set program counter to 0x200 because this is where programs will load
	opcode = 0;	// Reset opcode to 0
	I = 0;		// Reset I to 0
	
	// Clear display by turning off all pixels
	for (int i = 0; i < 2048; ++i) {
		graphics[i] = 0;
	}

	// Clear keypads and V registers
	for (int i = 0; i < 16; ++i) {
		key[i] = 0;
		V[i] = 0;
	}
	
	// Clear stack
	for (int i = 0; i < stack.size(); ++i) {
		stack.pop();
	}

	// Clear memory
	for (int i = 0; i < 4096; ++i) {
		memory[i] = 0;
	}

	// Load font into memory
	for (int i = 0; i < 80; ++i) {
		memory[i] = fontSet[i];
	}

	// Draw Flag
	drawFlag = false;

	// Reset timers
	delayTimer = 0;
	soundTimer = 0;

	// Seed RNG
	srand(time(NULL));
}

// Initialise and load ROM

bool Chip8::loadRom(const char *filePath) {

	// Initialise
	init();

	// Open ROM file
	FILE* rom = fopen(filePath, "rb");
	if (rom == NULL) {
		std::cerr << "ROM failed\n";
		return false;
	}

	// Points to the file and sets the position indicator to the end of the file
	fseek(rom, 0, SEEK_END);

	// ftell() finds the current position indicator value and it gets set to romSize
	long romSize = ftell(rom);

	// Sets the value of the position indicator to the beginning of the file
	rewind(rom);

	// Creating a buffer to allocate memory to store ROM
	char* romBuffer = (char*) malloc(sizeof(char) * romSize);
	if (romBuffer == NULL) {
		std::cerr << "Memory allocation failed\n";
		return false;
	}

	// Copy ROM into buffer
	size_t result = fread(romBuffer, 1, (size_t)romSize, rom);
	if (result != romSize) {
		std::cerr << "ROM read failed\n";
		return false;
	}

	// Copy buffer into memory
	if (4096 - 512 > romSize) {
		for (int i = 0; i < romSize; ++i) {
			memory[i+512] = (uint8_t) romBuffer[i];
		}
	}
	else {
		std::cerr << "ROM too large\n";
		return false;
	}

	// Cleanup
	fclose(rom);
	free(romBuffer);

	return true;
}


void Chip8::fdeCycle() {

	// Get opcode
	opcode = memory[pc] << 8 | memory[pc + 1]; 
	pc += 2;

	switch (opcode & 0xF000) {

		// Starts with 0
		case 0x0000: 
			switch (opcode & 0x000F) {
				// 00E0 clears the screen
				 case 0x0000:
					for (int i = 0; i < 2048; ++i) {
						graphics[i] = 0;
					}
					drawFlag = true;
					break;
				// 00EE returns from a subroutine
				 case 0x000E:
					pc = stack.peek();
					stack.pop();
					break;
			}
			break;
		
		// Starts with 1 (1NNN jumps to address NNN)
		case 0x1000:
			pc = opcode & 0x0FFF;
			//pc -= 2;
			break;

		// Starts with 2 (2NNN calls subroutine at NNN)
		case 0x2000:
			stack.push(pc);
			pc = opcode & 0x0FFF;
			break;

		// Starts with 3 (3XNN skips next instruction if VX == NN)
		case 0x3000: 
			if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
				pc += 2;
			break;

		// Starts with 4 (4XNN skips next instruction if VX != NN)
		case 0x4000:
			if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
				pc += 2;
			break;

		// Starts with 5 (5XY0 skips next instruction if VX == VY)
		case 0x5000: 
			if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
				pc += 2;
			break;

		// Starts with 6 (6XNN sets VX to NN)
		case 0x6000:
			V[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
			break;

		// Starts with 7 (7XNN adds NN to VX)
		case 0x7000: 
			V[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
			break;

		// Starts with 8
		case 0x8000:
			// 8XY[]
			switch (opcode & 0x000F) {
				// 8XY0 sets VX to VY's value
				case 0x0000:
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
					break;
				// 8XY1 sets VX to VX OR VY
				case 0x0001:
					V[(opcode & 0x0F00) >> 8] |= V[(opcode & 0x00F0) >> 4];
					break;
				// 8XY2 sets VX to VX AND VY
				case 0x0002:
					V[(opcode & 0x0F00) >> 8] &= V[(opcode & 0x00F0) >> 4];
					break;
				// 8XY3 sets VX to VX XOR VY
				case 0x0003:
					V[(opcode & 0x0F00) >> 8] ^= V[(opcode & 0x00F0) >> 4];
					break;
				// 8XY4 adds VY to VX (sets VF to 1 if carry)
				case 0x0004:
					V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];
					if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
						V[0xF] = 1;
					else
						V[0xF] = 0;
					break;
				// 8XY5 subtracts VY from VX (sets VF to 1 if NO borrow)
				case 0x0005:
					if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
						V[0xF] = 0;
					else
						V[0xF] = 1;

					V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
					break;
				// 8XY6 shifts VX right by one (sets VF to LSB before shift)
				case 0x0006:
					V[0xF] = V[(opcode & 0x0F00) >> 8] & 0x1;
					V[(opcode & 0x0F00) >> 8] >>= 1;
					break;
				// 8XY7 sets VX to VY - VX (sets VF to 1 if NO borrow)
				case 0x0007:
					if (V[(opcode & 0x00F0) >> 4] <= V[(opcode & 0x0F00) >> 8])
						V[0xF] = 0;
					else
						V[0xF] = 1;
					V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
					break;
				// 8XYE shifts VX left by one (sets VF to MSB before shift)
				case 0x000E:
					V[0xF] = V[(opcode & 0x0F00) >> 8] >> 7;
					V[(opcode & 0x0F00) >> 8] <<= 1;
					break;
			}
			break;

		// Starts with 9 (9XY0 skips next instruction if VX and VY not equal)
		case 0x9000: 
			if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
				pc += 2;
			break;

		// Starts with A (ANNN sets I to NNN)
		case 0xA000:
			I = opcode & 0x0FFF;
			break;

		// Starts with B (BNNN jumps to address NNN + V0)
		case 0xB000: 
			pc = (opcode & 0x0FFF) + V[0];
			//pc -= 2;
			break;

		// Starts with C (CXNN sets VX to random number and ANDS with NN)
		case 0xC000:
			// Generates random number and mods it by 256
			V[(opcode & 0x0F00) >> 8] = (rand() % (0xFF + 1)) & (opcode & 0x00FF);
			break;

		// Starts with D (DXYN draws to the screen)
		case 0xD000: {

			drawFlag = true;
			
			// Unsigned short used because the values can only be positive 16bit integers
			unsigned short xcoord,ycoord,height,pixel;

			// Defining x,y coordinates and the height of the sprite. 
			xcoord = V[(opcode & 0x0F00) >> 8] % 64; // Modulo used because it will
			ycoord = V[(opcode & 0x00F0) >> 4] % 32; // Allow sprites to wrap 
			height = opcode & 0x000F;

			V[0xF] = 0; // Setting VF to 0 before the for loop so if there is no change it stays 0
			
			// Setting a loop of N rows
			for (int row = 0; row < height; row++) {
				pixel = memory[I + row]; // Get Nth byte of sprite data


				// For each pixel in the current row 
				for (int bit = 0; bit < 8; bit++) { 
					if (pixel & (0x80 >> bit)) { // Checks if the current sprite pixel is on
								     
						// Checks the coordinate is on
						if (graphics[ (64 * (ycoord + row)) + (xcoord + bit) ]) 
							V[0xF] = 1;

						// Uses XOR to flip state between 1 and 0 
						graphics[ (64 * (ycoord + row)) + (xcoord + bit) ] ^= 1;
						
					}
				}
			}
		}	
		break;

		// Starts with E
		case 0xE000: 
			// EX[][]
			switch (opcode & 0x00FF) {
				// EX9E skips the next instruction if key in VX pressed
				case 0x009E:
					if (key[V[(opcode & 0x0F00) >> 8]])
						pc += 2;
					break;

				// EXA1 skips the next instruction if key in VX not pressed
				case 0x00A1:
					if (!key[V[(opcode & 0x0F00) >> 8]])
						pc += 2;
					break;
			}

			break;

		// Starts with F
		case 0xF000:
			// FX[][]
			switch (opcode & 0x00FF) {
				// FX07 sets VX to the delay timer value
				case 0x0007:
					V[(opcode & 0x0F00) >> 8] = delayTimer;
					break;
				// FX0A wait for A key press, store in VX
				case 0x000A:
					{bool keyPress = false;

					for (int i = 0; i < 16; ++i) {

						if (key[i]) {
							V[(opcode & 0x0F00) >> 8] = i;
							keyPress = true;
						}
					}
					
					// Redo the instruction if no key is pressed
					if (!keyPress)
						return;
					break;}
				// FX15 sets delay timer to VX
				case 0x0015:
					delayTimer = V[(opcode & 0x0F00) >> 8];
					break;
				// FX18 sets sound timer to VX
				case 0x0018:
					soundTimer = V[(opcode & 0x0F00) >> 8];
					break;
				// FX1E adds VX to I (sets VF to 1 if range overflow)
				case 0x001E:
					I += V[(opcode & 0x0F00) >> 8];
					if (I > 0xFFF)
						V[0xF] = 1;
					else
						V[0xF] = 0;
					break;
				// FX29 sets I to location of sprite for char VX 
				case 0x0029:
					// Multiplied by 5 because characters are stored by
					// a 4x5 font
					I = V[(opcode & 0x0F00) >> 8] * 0x5;
					break;
				// FX33 stores binary-coded decimal rep of VX at I to I + 2
				case 0x0033:
					// Puts each of the three digits individually
					// at the specified memory locations
					memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
					memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
					memory[I + 2] = V[(opcode & 0x0F00) >> 8] % 10;
					break;
				// FX55 stores registers V0 to VX in memory from address I
				case 0x0055:
					for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
						memory[I+i] = V[i];
					/* The original interpreter set I to I + X + 1 */ 
					//I += ((opcode & 0x0F00) >> 8) + 1;
					break;
				// FX65 stores registers V0 to VX in memory from address I
				case 0x0065:
					for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
						V[i] = memory[I+i];
					/* The original interpreter set I to I + X + 1 */ 
					//I += ((opcode & 0x0F00) >> 8) + 1;
					break;
			}
			break;

		default:
			printf("\nopcode unkown: 0x%.4X\n",opcode);
			exit(1);
	}

	// Timers
	
	if (delayTimer)
		--delayTimer;
	if (soundTimer)
		--soundTimer;
}
