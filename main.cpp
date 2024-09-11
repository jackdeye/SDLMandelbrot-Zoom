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
	std::mutex mtx;
	bool done = false;

	if (!init(context))
	{
		printf("Failed to initialize!\n");
		close(context);
		return 1;
	}

	//Coord start = Coord(0,0);
	//Coord end = Coord(PANEL_WIDTH, PANEL_HEIGHT);

	std::queue<panel*> animationQueue = simpleQueueInitVert();
	std::queue<std::function<void()>> taskQueue;

	std::vector<std::thread> threads;
	for (int i = 0; i < NUM_THREADS; i++) {
		threads.emplace_back(worker, std::ref(taskQueue), std::ref(mtx), std::ref(done));
	}
	{
		std::unique_lock<std::mutex> lock(mtx);
		while (!animationQueue.empty()) {
			panel* p = animationQueue.front();
			animationQueue.pop();
			auto boundFunction = [p, context]() {
				//std::unique_lock<std::mutex> lock(mtx);
				render_strip(context.renderer, p->TopLeft, p->BottomRight);
				delete p;
			};
			taskQueue.push(boundFunction);
		}
	}
	// Signal that no more tasks will be added
	{
		std::unique_lock<std::mutex> lock(mtx);
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
			SDL_RenderPresent(context.renderer);
		}
		SDL_RenderPresent(context.renderer);
	}
	close(context);
	for (auto& thread : threads) {
		thread.join();
	}

	return 0;
}

