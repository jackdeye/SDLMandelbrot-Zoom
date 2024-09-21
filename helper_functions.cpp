#include "helper_functions.h"
#include "helper_structs.h"
#include <queue>
template <typename T>
interval<T> ithSubInterval(interval<T> global, int numInts, int index) {
	//Assume the start for global < end for global, and both numInts and index are strictly positive
	//index is zero indexed
	T dist = global.end - global.start;
	T stepLen = dist / numInts;
	return interval<T>((index * stepLen) + global.start, ((index + 1) * stepLen) + global.start);
}

bool init(SDLContext& context)
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
		context.gWindow = SDL_CreateWindow("Mandelbrot Zoom", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
		if (context.gWindow == nullptr)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			//Create renderer for window
			context.renderer = SDL_CreateRenderer(context.gWindow, -1, SDL_RENDERER_ACCELERATED);
			if (context.renderer == nullptr)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor(context.renderer, 0xFF, 0xFF, 0xFF, 0xFF);
			}
			//queueInit();
		}
	}
	return success;
}

void close(SDLContext& context)
{
	//Destroy window
	SDL_DestroyWindow(context.gWindow);
	context.gWindow = nullptr;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

SDL_Surface* loadSurface(std::string path, SDLContext& context)
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
		optimizedSurface = SDL_ConvertSurface(loadedSurface, context.gScreenSurface->format, 0);
		if (optimizedSurface == nullptr)
		{
			printf("Unable to optimize image %s! SDL Error: %s\n", path.c_str(), SDL_GetError());
		}
		//Get rid of old loaded surface
		SDL_FreeSurface(loadedSurface);
	}

	return optimizedSurface;
}

std::queue<panel*> simpleQueueInit(interval<double> interX, interval<double> interY) {
	std::queue<panel*> panels;
	for (int i = 0; i < SCREEN_HEIGHT; i += PANEL_HEIGHT) {
		interval<double> subY = ithSubInterval<double>(interY, SCREEN_HEIGHT / PANEL_HEIGHT, i);
		for (int j = 0; j < SCREEN_WIDTH; j += PANEL_WIDTH) {
			interval<double> subX = ithSubInterval<double>(interX, SCREEN_WIDTH / PANEL_WIDTH, j);
			Coord start = Coord(j, i);
			Coord end = Coord(j + PANEL_WIDTH, i + PANEL_HEIGHT);
			panels.push(new panel(start, end, subX, subY));
		}
	}
	return panels;
}

std::queue<panel*> simpleQueueInitVert(interval<double> interX, interval<double> interY) {
	std::queue<panel*> panels;
	for (int i = 0; i * PANEL_WIDTH < SCREEN_WIDTH; i++) {
		interval<double> subX = ithSubInterval<double>(interX, SCREEN_WIDTH / PANEL_WIDTH, i);
		for (int j = 0; j * PANEL_HEIGHT < SCREEN_HEIGHT; j++) {
			interval<double> subY = ithSubInterval<double>(interY, SCREEN_HEIGHT / PANEL_HEIGHT, j);
			Coord start = Coord(i*PANEL_WIDTH, j*PANEL_HEIGHT);
			Coord end = Coord((i + 1) * PANEL_WIDTH, (j + 1) * PANEL_HEIGHT);
			panels.push(new panel(start, end, subX, subY));
		}
	}
	return panels;
}

SDL_Texture* createTextureFromPixels(SDL_Renderer* renderer, color *pixels, int width, int height) {
	SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(pixels, width, height, sizeof(color), width * sizeof(color),
		0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
	return texture;
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
/*
void queueInit() {
	const int numTall = SCREEN_HEIGHT / PANEL_HEIGHT;
	const int numWide = SCREEN_WIDTH / PANEL_WIDTH;
	panel* panelGrid[numTall][numWide]{};
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
*/