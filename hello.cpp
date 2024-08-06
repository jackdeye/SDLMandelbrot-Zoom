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
SDL_Renderer* renderer = nullptr;
std::queue<panel*> animationQueue;
std::mutex mtx;

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
		
	std::vector<std::thread> threads;
	for (int i = 0; i < NUM_THREADS; i++) {
		//threads.emplace_back();
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

		}
	}
	close();
	return 0;
}

//The following is entirely unneccesary; however, I wanted the spiralling outward pattern that Cinebench has, 
//So I decided to program it myself. The following grid below should be drawn in the following order:
/*
-----------------
| 10| 3 | 2 | 9 |
-----------------
| 11| 4 | 1 | 8 |
-----------------
| 12| 5 | 6 | 7 |
-----------------
*/

void queueInit() {
	const int numTall = SCREEN_HEIGHT / PANEL_HEIGHT;
	const int numWide = SCREEN_WIDTH / PANEL_WIDTH;
	panel* panelGrid[numTall][numWide];
	for (int i = 0; i < SCREEN_HEIGHT; i += PANEL_HEIGHT) {
		for (int j = 0; j < SCREEN_WIDTH; j += PANEL_WIDTH) {
			panelGrid[i / PANEL_HEIGHT][j / PANEL_WIDTH] = new panel(j, i, j + PANEL_WIDTH, i + PANEL_HEIGHT);
		}
	}
	int numPanels = numTall * numWide;
	int targetX = SCREEN_WIDTH / 2;
	targetX -= targetX % PANEL_WIDTH;
	int targetY = SCREEN_HEIGHT / 2;
	targetY -= targetY % PANEL_HEIGHT;
	int size = 0;
	int xIndex = targetX / PANEL_WIDTH, yIndex = targetY / PANEL_HEIGHT;
	bool doneSpiraling = false;
	while (size < numTall || size < numWide) {
		size++;
		animationQueue.push(panelGrid[yIndex][xIndex]);
		
		//Up
		for (int i = 0; i < size; i++) {
			yIndex--;
			if (yIndex < 0) {
				doneSpiraling = true;
				break;
			}
			animationQueue.push(panelGrid[yIndex][xIndex]);
		}
		if (doneSpiraling) break;
		//Left
		for (int i = 0; i < size; i++) {
			xIndex--;
			if (xIndex < 0) {
				doneSpiraling = true;
				break;
			}
			animationQueue.push(panelGrid[yIndex][xIndex]);
		}
		if (doneSpiraling) break;
		size++;
		//Down
		for (int i = 0; i < size; i++) {
			yIndex++;
			animationQueue.push(panelGrid[yIndex][xIndex]);
		}
		//Right
		for (int i = 0; i < size; i++) {
			xIndex++;
			animationQueue.push(panelGrid[yIndex][xIndex]);
		}
	}

	//Do we have any to the right or left
}