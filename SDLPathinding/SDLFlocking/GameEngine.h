#pragma once
#include "SDL.h"
#include "SDL_TTF.h"
#include <vector>
#include "GridFactory.h"


class GameEngine
{

public:

	GameEngine();//startupData will be a .JSON file location that the application was started with (e.g. if a file is dragged onto the exe)
	~GameEngine();

	void Init(const char* name, int xPosition, int yPosition, int width, int height, bool isFullscreen);

	void HandleEvents(SDL_Event* e);

	void Update();

	void Draw();



	void Clean();

	bool IsRunning() { return gameIsRunning; }


private:

	void CapFrameRate(int frameRate);


	SDL_Event * e;

	SDL_Window *window;
	SDL_Renderer *renderer;
	bool gameIsRunning = false;

	float screenScale;
	int screenWidth, screenHeight;

	SDL_Color backColor;


	GridFactory* _gridFactory;

	std::vector<GameObject*> gameObjects;
	GOBoid* NPC;
	GOBoid* NPC2;


	int frameRate;
	float FRControl;//e.g. 16.67MS at 60fps, calculated at runtime, used to lock framerate at correct rate.

	int rightClickIndex;

};