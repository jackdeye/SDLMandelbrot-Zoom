#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <thread>
#include <mutex>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//Starts up SDL and creates window
bool init();
//Loads media
bool loadMedia();
//Frees media and shuts down SDL
void close();
//Renders a section of the mandelbrot image
void render_strip(SDL_Surface* surface, int start_y, int end_y);

//Loads individual image
SDL_Surface* loadSurface(std::string path);
//The window we'll be rendering to
SDL_Window* gWindow = nullptr;
//The surface contained by the window
SDL_Surface* gScreenSurface = nullptr;
//Current displayed PNG image
SDL_Surface* gPNGSurface = nullptr;

bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		//Create window
		gWindow = SDL_CreateWindow("SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == nullptr)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Initialize PNG loading
			int imgFlags = IMG_INIT_PNG;
			if (!(IMG_Init(imgFlags) & imgFlags))
			{
				printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
				success = false;
			}
			else
			{
				gScreenSurface = SDL_GetWindowSurface(gWindow);
			}
		}
	}

	return success;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//Load PNG surface
	gPNGSurface = loadSurface("Resources/up.jpg");
	if (gPNGSurface == nullptr)
	{
		printf("Failed to load PNG image!\n");
		success = false;
	}

	return success;
}

void close()
{
	//Free loaded image
	SDL_FreeSurface(gPNGSurface);
	gPNGSurface = nullptr;

	//Destroy window
	SDL_DestroyWindow(gWindow);
	gWindow = nullptr;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

SDL_Surface* loadSurface(std::string path)
{
	//The final optimized image
	SDL_Surface* optimizedSurface = nullptr;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());
	if (loadedSurface == nullptr)
	{
		printf("Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError());
	}
	else
	{
		//Convert surface to screen format
		optimizedSurface = SDL_ConvertSurface(loadedSurface, gScreenSurface->format, 0);
		if (optimizedSurface == nullptr)
		{
			printf("Unable to optimize image %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}
		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	return optimizedSurface;
}

void render_strip(SDL_Surface* surface, int start_y, int end_y) {
	
}

int main(int argc, char* args[])
{
	//Creates window
	if (!init())
	{
		printf("Failed to initialize!\n");
		close();
		return 1;
	}
		
	std::mutex mutex;
	std::vector<std::thread> threads;

	// ... divide image into strips and create threads

	for (auto& thread : threads) {
		thread.join();
	}

	bool quit = false;
	SDL_Event e;

	while (!quit)
	{
		while (SDL_PollEvent(&e) != 0)
		{		
			//User requests quit
			if (e.type == SDL_QUIT)
			{
					quit = true;
			}
		SDL_BlitSurface(gPNGSurface, NULL, gScreenSurface, NULL);
		SDL_UpdateWindowSurface(gWindow);
		}
	}
	close();
	return 0;
}
