#pragma once

#include <iostream>

#include "SDL.h"
#include "GameEngine.h"

int main(int argc, char* argv[])
{

	GameEngine *engineMain = new GameEngine();
	while (engineMain->IsRunning())
	{
		engineMain->Update();
		engineMain->Draw();
	}

	engineMain->Clean();
	SDL_Quit();
	TTF_Quit();

	return 0;

}