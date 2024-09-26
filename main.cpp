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

void createRenederedTile(SDL_Renderer* renderer, panel* inputPanel, std::queue<panel*>& renderedQueue, std::mutex& mtx) {
	std::ostringstream oss1;
	oss1 << std::this_thread::get_id() << " has started drawing from (" << inputPanel->topLeftPixelPos.x << ", " << inputPanel->topLeftPixelPos.y << ") to (" << inputPanel->bottomRightPixelPos.x << ", " << inputPanel->bottomRightPixelPos.y << ")" << std::endl;
	std::cout << oss1.str() << std::endl;
	
	color* tile = new color[PANEL_WIDTH*PANEL_HEIGHT];
	for (int i = 0; i < PANEL_HEIGHT; i++) {
		for (int j = 0; j < PANEL_WIDTH; j++) {
			tile[i * PANEL_WIDTH + j].r = 0xFF;
		}
	}

	SDL_Surface* surface = SDL_CreateRGBSurfaceFrom(
		tile, PANEL_WIDTH, PANEL_HEIGHT, 24, PANEL_WIDTH * sizeof(color),
		0x000000FF, 0x0000FF00, 0x00FF0000, 0
	);
	delete[] tile;

	//Want to throw error, think about this later
	if (!surface) {
		SDL_Log("SDL_CreateRGBSurfaceFrom failed: %s", SDL_GetError());
		return;
	}

	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface); 
	inputPanel->textureToBeRendered = texture;
	{
		std::unique_lock<std::mutex> lock(mtx);
		renderedQueue.push(inputPanel);
	}
	std::ostringstream oss2;
	oss2 << std::this_thread::get_id() << " has finished drawing from (" << inputPanel->topLeftPixelPos.x << ", " << inputPanel->topLeftPixelPos.y << ") to (" << inputPanel->bottomRightPixelPos.x << ", " << inputPanel->bottomRightPixelPos.y << ")" << std::endl;
	std::cout << oss2.str() << std::endl;
	//std::this_thread::sleep_for(std::chrono::seconds(1));

}


static void worker(std::queue<std::function<void()>>& tasks, std::mutex& mtx, bool& done) {
	while (true) {
		std::function<void()> task;
		{
			std::unique_lock<std::mutex> lock(mtx);
			if (!tasks.empty()) {
				//front() returns a reference (lvalue) but std::move casts it to a rvalue ref, 
				//meaning the move operator can be called and not the copy constructor
				task = std::move(tasks.front());
				tasks.pop();
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
	SDLContext context;
	bool done = false;

	interval<double> xint = interval<double>(-2.0,2.0), yint = interval<double>(-2.0,2.0);

	if (!init(context))
	{
		printf("Failed to initialize!\n");
		close(context);
		return 1;
	}

	std::mutex mtxPre, mtxPost;
	std::queue<panel*> toBeCalculatedQueue = simpleQueueInitVert(xint,yint); 
	std::queue<std::function<void()>> inputTaskQueue; //mtxPre is associated with this queue
	std::queue<panel*> toBeRenderedQueue; //mtxPost is associated with this queue

	std::vector<std::thread> threads;
	for (int i = 0; i < NUM_THREADS; i++) {
		//std::ref is used to pass arguments by reference to the worker function when creating a new thread
		threads.emplace_back(worker, std::ref(inputTaskQueue), std::ref(mtxPre), std::ref(done));
	}
	{
		std::unique_lock<std::mutex> lock(mtxPre);
		while (!toBeCalculatedQueue.empty()) {
			panel* panelPtr = toBeCalculatedQueue.front();
			toBeCalculatedQueue.pop();
			auto boundFunction = [panelPtr, context, &toBeRenderedQueue, &mtxPost]() {
				createRenederedTile(context.renderer, panelPtr, toBeRenderedQueue, mtxPost);
			};
			inputTaskQueue.push(boundFunction);
		}
	}
	// Signal that no more tasks will be added
	{
		std::unique_lock<std::mutex> lock(mtxPre);
		done = true;
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
			std::cout << "We here" << std::endl;
		}
		SDL_Texture* texture = nullptr;
		SDL_Rect dstRect;
		{
			std::unique_lock<std::mutex> lock(mtxPost);
			if (!toBeRenderedQueue.empty())
			{
				texture = std::move(toBeRenderedQueue.front()->textureToBeRendered);
				dstRect = { toBeRenderedQueue.front()->topLeftPixelPos.x, toBeRenderedQueue.front()->topLeftPixelPos.y, PANEL_WIDTH, PANEL_HEIGHT };
				toBeRenderedQueue.pop();
				std::cout << "Drew a square" << std::endl;

			}
		}

		if (texture)
		{
			if (SDL_RenderCopy(context.renderer, texture, NULL, &dstRect) != 0)
			{
				std::cerr << "SDL_RenderCopy Error: " << SDL_GetError() << std::endl;
			}
			//SDL_DestroyTexture(texture);
		}

		SDL_RenderPresent(context.renderer);
	}
	close(context);
	for (auto& thread : threads) {
		thread.join();
	}

	return 0;
}

