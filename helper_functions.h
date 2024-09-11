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

std::queue<panel*> simpleQueueInit();

std::queue<panel*> simpleQueueInitVert();

#endif // !FUNCTIONS
