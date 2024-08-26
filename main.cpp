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

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int PANEL_WIDTH = 32;
const int PANEL_HEIGHT = 32;
const int NUM_THREADS = 3; //This can be changed later

struct Coord {
	Coord() {};
	Coord(int x, int y): x(x), y(y){}
	int x = 0;
	int y = 0;
};

struct panel{
	panel() {};
	panel(Coord TopLeft, Coord BottomRight) : TopLeft(TopLeft), BottomRight(BottomRight){}
	Coord TopLeft, BottomRight;
};


SDL_Window* gWindow = nullptr;	//The window we'll be rendering to
SDL_Surface* gScreenSurface = nullptr;	//The surface contained by the window
SDL_Renderer* renderer = nullptr;
std::queue<panel*> animationQueue;
std::queue<std::function<void()>> taskQueue;
std::mutex mtx;
std::condition_variable cv;
bool done = false;


bool init();
void close();
//void queueInit();
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
			//Create renderer for window
			renderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED);
			if (renderer == nullptr)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
			}
			//queueInit();
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
std::queue<panel*> simpleQueueInit(std::queue<panel*> panels);
std::queue<panel*> simpleQueueInitVert(std::queue<panel*> panels);


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

void render_strip(SDL_Renderer* renderer, Coord start, Coord end) {
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
	for (int i = start.y; i < end.y; i++) {
		for (int j = start.x; j < end.x; j++) {
			SDL_RenderDrawPoint(renderer, j, i);
		}
	}
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::ostringstream oss;
	oss << std::hash<std::thread::id>{}(std::this_thread::get_id()) << " has finished drawing from (" << start.x << ", " << start.y << ") to (" << end.x << ", " << end.y << ")" << std::endl;
	std::cout << oss.str() << std::endl;
}

void worker() {
	while (true) {
		std::function<void()> task;
		{
			std::unique_lock<std::mutex> lock(mtx);
			cv.wait(lock, [] { return !taskQueue.empty() || done; });
			if (!taskQueue.empty()) {
				//front() returns a reference (lvalue) but std::move casts it to a rvalue ref, 
				//meaning the move operator can be called and not the copy constructor
				task = std::move(taskQueue.front());
				taskQueue.pop();
			}
			else if (done) {
				break;
			}
		}
		if (task) {
			task();
		}
	}
}

int main(int argc, char* args[])
{
	if (!init())
	{
		printf("Failed to initialize!\n");
		close();
		return 1;
	}

	//Coord start = Coord(0,0);
	//Coord end = Coord(PANEL_WIDTH, PANEL_HEIGHT);

	animationQueue = simpleQueueInitVert(animationQueue);

	std::vector<std::thread> threads;
	for (int i = 0; i < NUM_THREADS; i++) {
		threads.emplace_back(worker);
	}
	{
		std::unique_lock<std::mutex> lock(mtx);
		while (!animationQueue.empty()) {
			panel* p = animationQueue.front();
			animationQueue.pop();
			auto boundFunction = [p]() {
				//std::unique_lock<std::mutex> lock(mtx);
				render_strip(renderer, p->TopLeft, p->BottomRight);
				delete p;
			};
			taskQueue.push(boundFunction);
		}
		cv.notify_all();
	}
	// Signal that no more tasks will be added
	{
		std::unique_lock<std::mutex> lock(mtx);
		done = true;
		cv.notify_all();
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
			SDL_RenderPresent(renderer);
		}
		SDL_RenderPresent(renderer);
	}
	close();
	
	for (auto& thread : threads) {
		thread.join();
	}
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

std::queue<panel*> simpleQueueInit(std::queue<panel*> panels) {
	for (int i = 0; i < SCREEN_HEIGHT; i+=PANEL_HEIGHT) {
		for (int j = 0; j < SCREEN_WIDTH; j+=PANEL_WIDTH) {
			Coord start = Coord(j,i);
			Coord end = Coord(j + PANEL_WIDTH, i + PANEL_HEIGHT);
			panels.push(new panel(start, end));
		}
	}
	return panels;
}

std::queue<panel*> simpleQueueInitVert(std::queue<panel*> panels) {
	for (int i = 0; i < SCREEN_WIDTH; i += PANEL_WIDTH) {
		for (int j = 0; j < SCREEN_HEIGHT; j += PANEL_HEIGHT) {
			Coord start = Coord(i, j);
			Coord end = Coord(i + PANEL_WIDTH, j + PANEL_HEIGHT);
			panels.push(new panel(start, end));
		}
	}
	return panels;
}
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