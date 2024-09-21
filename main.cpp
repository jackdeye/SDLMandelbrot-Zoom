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
	color* tile = new color[PANEL_WIDTH*PANEL_HEIGHT];
	for (int i = 0; i < PANEL_HEIGHT; i++) {
		for (int j = 0; j < PANEL_WIDTH; j++) {
			tile[i * PANEL_WIDTH + j].r = 0xFF;
		}
	}
	SDL_Texture* texture = createTextureFromPixels(renderer, tile, PANEL_WIDTH, PANEL_HEIGHT);
	inputPanel->textureToBeRendered = texture;
	
	{
		std::unique_lock<std::mutex> lock(mtx);
		renderedQueue.push(inputPanel);
	}

	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::ostringstream oss;
	oss << std::hash<std::thread::id>{}(std::this_thread::get_id()) << " has finished drawing from (" << inputPanel->topLeftPixelPos.x << ", " << inputPanel->topLeftPixelPos.y << ") to (" << inputPanel->bottomRightPixelPos.x << ", " << inputPanel->bottomRightPixelPos.y << ")" << std::endl;
	std::cout << oss.str() << std::endl;
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
				//std::unique_lock<std::mutex> lock(mtx);
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
			
			SDL_Texture* texture = std::move(toBeRenderedQueue.front()->textureToBeRendered);
			SDL_Rect dstRect = { toBeRenderedQueue.front()->topLeftPixelPos.x, toBeRenderedQueue.front()->topLeftPixelPos.y, PANEL_WIDTH, PANEL_HEIGHT};
			SDL_RenderCopy(context.renderer, texture, NULL, &dstRect);
		}
		SDL_RenderPresent(context.renderer);
	}
	close(context);
	for (auto& thread : threads) {
		thread.join();
	}

	return 0;
}

