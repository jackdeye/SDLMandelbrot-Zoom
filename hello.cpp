#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <chrono>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int PANEL_WIDTH = 32;
const int PANEL_HEIGHT = 32;
const int NUM_THREADS = 1; //This can be changed later

SDL_Window* gWindow = nullptr;	//The window we'll be rendering to
SDL_Surface* gScreenSurface = nullptr;	//The surface contained by the window
//Going to want to use SDL_Renderer and RenderClear, RenderCopy, RenderPresent
//SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
std::queue<panel*> animationQueue;

struct panel {
	panel() {}
	panel(int x1, int y1, int x2, int y2) : 
		topLeftX(x1), topLeftY(y1), bottomRightX(x1), bottomRightY(y2) {}
	int topLeftX, topLeftY, bottomRightX, bottomRightY;
};

bool init();
void close();
void queueInit();

bool init()
{
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
		gWindow = SDL_CreateWindow("Mandelbrot Zoom", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (gWindow == nullptr)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			queueInit();
		}
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
		
	std::mutex locks[NUM_THREADS];
	std
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

void queueInit() {
	panel* panelGrid[SCREEN_HEIGHT / PANEL_HEIGHT][SCREEN_WIDTH / PANEL_WIDTH];
	for (int i = 0; i < SCREEN_HEIGHT; i += PANEL_HEIGHT) {
		for (int j = 0; j < SCREEN_WIDTH; j += PANEL_WIDTH) {
			panelGrid[i / PANEL_HEIGHT][j / PANEL_WIDTH] = new panel(j, i, j + PANEL_WIDTH, i + PANEL_HEIGHT);
		}
	}
	int numPanels = SCREEN_HEIGHT / PANEL_HEIGHT * SCREEN_WIDTH / PANEL_WIDTH;
	int targetX = SCREEN_WIDTH / 2;
	targetX -= targetX % PANEL_WIDTH;
	int targetY = SCREEN_HEIGHT / 2;
	targetY -= targetX % PANEL_HEIGHT;
	int size = 1;
	while (animationQueue.size() < numPanels) {
		animationQueue.push(panelGrid[targetY / PANEL_HEIGHT][targetX / PANEL_WIDTH]);
		for (int i = 0; i < size; i++) {
			targetX
				animationQueue.push(panelGrid[targetY / PANEL_HEIGHT][targetX / PANEL_WIDTH]);
		}
	}
}