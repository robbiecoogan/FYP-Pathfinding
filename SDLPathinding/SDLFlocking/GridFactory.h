#pragma once

#include "SDL.h"
#include "GameObject.h"

class GridFactory
{
public:
	GridFactory(SDL_Window* _window, SDL_Renderer* _renderer, int _screenWidth, int _screenHeight);
	GridFactory(SDL_Window* _window, SDL_Renderer* _renderer, int _screenWidth, int _screenHeight, int cellWidth, int cellHeight);
	~GridFactory();

	void Update();
	void Draw();

	std::vector<GOGridNode*> CreateGrid();

private:
	SDL_Window* window;
	SDL_Renderer* renderer;

	std::vector<GOGridNode*> copyGrid;

	float screenWidth, screenHeight;
	float cellWidth, cellHeight;

	static int defaultCellWidth, defaultCellHeight;

};
