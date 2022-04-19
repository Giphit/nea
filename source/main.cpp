#include <iostream>
#include <chrono>
#include <thread>
#include <stdint.h>
#include <stdio.h>
#include "SDL2/SDL.h"

#include "chip8.h"

using namespace std;

uint8_t keymap[16] = {
	SDLK_x,
	SDLK_1,
	SDLK_2,
	SDLK_3,
	SDLK_q,
	SDLK_w,
	SDLK_e,
	SDLK_a,
	SDLK_s,
	SDLK_d,
	SDLK_z,
	SDLK_c,
	SDLK_4,
	SDLK_r,
	SDLK_f,
	SDLK_v,
};

int main(int argc, char** argv) {
	// Make sure the program is run correctly
	if (argc != 2) {
		cout << "Usage: main [rom-file]\n";
		return 1;
	}
	
	// Instance of chip8 class
	Chip8 chip8 = Chip8();

	int width = 1024, height = 512; // Dimensions of the window
	
	// Window pointer
	SDL_Window* window = NULL;
	
	// Starting SDL and returning error if something goes wrong
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		printf("SDL Initialisation Failed. Error: %s\n", SDL_GetError());
		exit(1);
	}

	// Create Window and return error if something goes wrong
	window = SDL_CreateWindow (
			"CHIP8-EMU",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			width,
			height,
			SDL_WINDOW_SHOWN
		);
	if (window == NULL) {
		printf("Window could not be created. Error: %s\n",SDL_GetError());
		exit(1);
	}

	// Create renderer
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_RenderSetLogicalSize(renderer,width,height);

	// Create texture to store frame buffer.
	SDL_Texture* texture = SDL_CreateTexture (
			renderer,
			SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING,
			64,32
		);

	// Temp pixel buffer
	uint32_t pixels[2048];
	
	if (!chip8.loadRom(argv[1]))
		return 2;

	// Emulation loop
	while (true) {
		chip8.fdeCycle();

		// SDL events
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT)
				exit(0);

			// Keydown events
			if (event.type == SDL_KEYDOWN) {
				for (int i = 0; i < 16; ++i) {
					if (event.key.keysym.sym == keymap[i])
						chip8.key[i] = 1;
				}
			}

			// Keyup events
			if (event.type == SDL_KEYUP) {
				for (int i = 0; i < 16; ++i) {
					if (event.key.keysym.sym == keymap[i])
						chip8.key[i] = 0;
				}
			}
		}


		// Redraw screen if draw occurs

		if (chip8.drawFlag) {
			chip8.drawFlag = false;

			// Store pixels in temporary buffer
			for (int i = 0; i < 2048; ++i) {
				uint8_t pixel = chip8.graphics[i];
				pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;
			}
			
			// Update SDL textures
			SDL_UpdateTexture(texture,NULL,pixels,64*sizeof(Uint32));

			// Clear screen then render
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer,texture,NULL,NULL);
			SDL_RenderPresent(renderer);
		}
		std::this_thread::sleep_for(std::chrono::microseconds(1600));
	}
}
