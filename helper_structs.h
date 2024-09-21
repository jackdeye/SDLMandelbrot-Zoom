#ifndef STRUCTS
#define STRUCTS

#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include <functional>
#include <mutex>
#include <queue>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int PANEL_WIDTH = 32;
const int PANEL_HEIGHT = 32;
const int NUM_THREADS = 3; //This can be changed later

struct Coord {
	Coord() {};
	Coord(int x, int y) : x(x), y(y) {}
	int x = 0;
	int y = 0;
};

template <typename T>
struct interval {
	interval() {};
	interval(T s, T e) : start(s), end(e) {}
	T start = 0;
	T end = 0;
};

struct panel {
	panel() {};
	panel(Coord topLeft, Coord bottomRight, interval<double> x, interval<double> y) 
		: topLeftPixelPos(topLeft), bottomRightPixelPos(bottomRight), xInterval(x), yInterval(y) {}
	Coord topLeftPixelPos, bottomRightPixelPos;
	interval<double> xInterval, yInterval;
	SDL_Texture* textureToBeRendered = nullptr;
};

struct color {
	Uint8 r = 0x00, g = 0x00, b = 0x00;
};

struct SDLContext {
	SDL_Window* gWindow = nullptr;	//The window we'll be rendering to
	SDL_Surface* gScreenSurface = nullptr;	//The surface contained by the window
	SDL_Renderer* renderer = nullptr;
};

#endif // !STRUCTS