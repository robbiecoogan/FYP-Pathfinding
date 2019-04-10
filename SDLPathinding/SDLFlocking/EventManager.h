#pragma once

#include "SDL.h"
#include <vector>

class EventManager
{
public:
	static EventManager& getInstance()
	{
		static EventManager instance;
		return instance;
		thisFrame = SDL_GetPerformanceCounter();
		lastFrame = thisFrame;
	}

	EventManager(EventManager const&) = delete;
	void operator=(EventManager const&) = delete;

	void UpdateEventList();
	std::vector<SDL_Event*> GetEvent(SDL_EventType eType);
	std::vector<SDL_Event*> GetEvent(SDL_WindowEventID eType);

	static float dTime;
	static float screenScale;

private:
	EventManager() {}

	std::vector<SDL_Event*> eventList;

	static double thisFrame, lastFrame;
};