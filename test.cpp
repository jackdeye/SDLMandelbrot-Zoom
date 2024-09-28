#include <SDL.h>
#include <sstream>
#include <SDL_image.h>
#include <stdio.h>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <functional>
#include <chrono>
#include <iostream>
#include "helper_structs.h"
#include "helper_functions.h"

int main(int argc, char* args[]) {
	SDLContext context;
	if (!init(context))
	{
		printf("Failed to initialize!\n");
		close(context);
		return 1;
	}
	color* tile1 = new color[PANEL_WIDTH * PANEL_HEIGHT];
	for (int i = 0; i < PANEL_HEIGHT; i++) {
		for (int j = 0; j < PANEL_WIDTH; j++) {
			tile1[i * PANEL_WIDTH + j].r = 0xFF;
		}
	}
	color* tile2 = new color[PANEL_WIDTH * PANEL_HEIGHT];
	for (int i = 0; i < PANEL_HEIGHT; i++) {
		for (int j = 0; j < PANEL_WIDTH; j++) {
			tile2[i * PANEL_WIDTH + j].b = 0xFF;
		}
	}

	SDL_Event e;
	bool quit = false;
	while (!quit)
	{
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}
		}
		SDL_Rect dstRect;
		if (tile1) {
			SDL_Surface* surface1 = SDL_CreateRGBSurfaceFrom(
				tile1, PANEL_WIDTH, PANEL_HEIGHT, 24, PANEL_WIDTH * sizeof(color),
				0x000000FF, 0x0000FF00, 0x00FF0000, 0
			);
			SDL_Texture* texture1 = SDL_CreateTextureFromSurface(context.renderer, surface1);
			SDL_FreeSurface(surface1);


			SDL_Rect dstRect = { 0, 0, PANEL_WIDTH, PANEL_HEIGHT };
			if (SDL_RenderCopy(context.renderer, texture1, NULL, &dstRect) != 0)
			{
				std::cerr << "SDL_RenderCopy Error: " << SDL_GetError() << std::endl;
			}
			SDL_DestroyTexture(texture1);
			delete[] tile1;
			tile1 = nullptr;
		}
		SDL_RenderPresent(context.renderer);

		if (tile2) {
			SDL_Surface* surface2 = SDL_CreateRGBSurfaceFrom(
				tile2, PANEL_WIDTH, PANEL_HEIGHT, 24, PANEL_WIDTH * sizeof(color),
				0x000000FF, 0x0000FF00, 0x00FF0000, 0
			);
			SDL_Texture* texture2 = SDL_CreateTextureFromSurface(context.renderer, surface2);
			SDL_FreeSurface(surface2);

			dstRect = { 0, 32, PANEL_WIDTH, PANEL_HEIGHT };
			if (SDL_RenderCopy(context.renderer, texture2, NULL, &dstRect) != 0)
			{
				std::cerr << "SDL_RenderCopy Error: " << SDL_GetError() << std::endl;
			}
			SDL_DestroyTexture(texture2);

			delete[] tile2;
			tile2 = nullptr;
		}
		SDL_RenderPresent(context.renderer);
	}
	close(context);



}
