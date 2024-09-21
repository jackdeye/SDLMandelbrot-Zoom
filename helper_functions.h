#ifndef FUNCTIONS
#define FUNCTIONS

#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include <queue>
#include "helper_structs.h"

bool init(SDLContext& context);

void close(SDLContext& context);

template <typename T>
interval<T> ithSubInterval(interval<T> global, int numInts, int index);

std::queue<panel*> simpleQueueInit(interval<double> interX, interval<double> interY);

std::queue<panel*> simpleQueueInitVert(interval<double> interX, interval<double> interY);

SDL_Texture* createTextureFromPixels(SDL_Renderer* renderer, color* pixels, int width, int height);


#endif // !FUNCTIONS
