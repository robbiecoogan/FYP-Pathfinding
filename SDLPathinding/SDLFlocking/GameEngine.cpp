#include "GameEngine.h"
#include <fstream>
#include <iostream>

#include <chrono>
#include <thread>

#include "EventManager.h"

GameEngine::GameEngine()
{

	screenScale = EventManager::screenScale;//
	frameRate = 60;


	FRControl = (float)(1.0f / frameRate);
	FRControl *= 1000;

	rightClickIndex = -1;

	//initalize SDL
	Init("Game Engine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, false);
}

GameEngine::~GameEngine()
{
}

void GameEngine::Init(const char * name, int xPosition, int yPosition, int width, int height, bool isFullscreen)
{
	backColor = { 200, 200, 255, 255 };

	if (SDL_Init(SDL_INIT_EVERYTHING) == 0 && TTF_Init() == 0)
	{
		int screenFlag = 0;
		if (isFullscreen) screenFlag = SDL_WINDOW_FULLSCREEN_DESKTOP;

		window = SDL_CreateWindow(name, xPosition, yPosition, width, height, screenFlag);

		renderer = SDL_CreateRenderer(window, -1, 0);

		SDL_SetRenderDrawColor(renderer, backColor.r, backColor.g, backColor.b, backColor.a);

		SDL_RenderSetScale(renderer, (1/screenScale), (1/screenScale));

		gameIsRunning = true;

		//SDL_SetWindowBordered(window, SDL_FALSE);

		e = new SDL_Event;
	}
	else
	{
		gameIsRunning = false;
	}
	
	NPC = new GOBoid(Vec2(300 * screenScale, 300 * screenScale), window, renderer);
	NPC->SetSprite("Assets\\EnemyDefault.png");
	NPC->destRect.w *= screenScale;
	NPC->destRect.h *= screenScale;
	gameObjects.emplace_back(NPC);

	//NPC2 = new GOBoid(Vec2(400 * screenScale, 50 * screenScale), window, renderer);
	//NPC2->SetSprite("Assets\\EnemyDefault.png");
	//NPC2->destRect.w *= screenScale;
	//NPC2->destRect.h *= screenScale;
	//gameObjects.emplace_back(NPC2);

	//get all grid nodes, and add them to the gameObject List.
	_gridFactory = new GridFactory(window, renderer, width, height);
	std::vector<GOGridNode*> searchGrid = _gridFactory->CreateGrid();
	//int gridSize = searchGrid.size();
	//for (size_t i = 0; i < gridSize; i++)
	//{
	//	gameObjects.emplace_back(searchGrid[i]);
	//}

	Vec2 toLoc(width - 200, height - 200);

	NPC->SetSearchSpace(searchGrid);
	//NPC2->SetSearchSpace(searchGrid);
	//NPC->Pathfind(toLoc, searchGrid);

}

void GameEngine::HandleEvents(SDL_Event * e)
{
	EventManager::getInstance().UpdateEventList();
	std::vector<SDL_Event*> tempE = EventManager::getInstance().GetEvent(SDL_WINDOWEVENT);
	int eSize = tempE.size();
	if (eSize > 0)
	{
		for (size_t i = 0; i < eSize; i++)
		{
			if (tempE[i]->window.event == SDL_WINDOWEVENT_CLOSE)
				gameIsRunning = false;
		}
	}
}

void GameEngine::Update()
{
	SDL_Event e;
	HandleEvents(&e);

	CapFrameRate(frameRate);

	int mouseX, mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);
	mouseX *= screenScale;
	mouseY *= screenScale;

	std::vector<SDL_Event*> temp = EventManager::getInstance().GetEvent(SDL_MOUSEBUTTONDOWN);
	int tempSize = temp.size();

	bool hasLeftClicked = false;

	for (size_t i = 0; i < gameObjects.size(); i++)
	{
		gameObjects[i]->Update();
	}

	_gridFactory->Update();

	for (size_t j = 0; j < tempSize; j++)
	{
		if (temp[j]->button.button == SDL_BUTTON_LEFT) hasLeftClicked = true;

		for (size_t i = 0; i < gameObjects.size(); i++)
		{
			
			GameObject* currObj = gameObjects[i];

			if (hasLeftClicked && gameObjects[i]->myType == GOType::NODE && ((GONode*)gameObjects[i])->IsCurrentlyClicked()) hasLeftClicked = false;


			if (temp[j]->button.button == SDL_BUTTON_RIGHT && mouseX > currObj->destRect.x && mouseX < currObj->destRect.x + currObj->destRect.w && mouseY > currObj->destRect.y && mouseY < currObj->destRect.y + currObj->destRect.h && currObj->myType == GOType::NODE)
			{
				if (rightClickIndex == -1)//if first right click
				{
					rightClickIndex = i;
				}
				else if (rightClickIndex != i)//if second right click
				{
					((GONode*)currObj)->AddConnection((GONode*)(gameObjects[rightClickIndex]));
					rightClickIndex = -1;
				}
			}
		}
	}


	/*if (hasLeftClicked)
	{
		GONode* newNode = new GONode(Vec2(mouseX, mouseY), window, renderer);
		newNode->destRect.w *= screenScale;
		newNode->destRect.h *= screenScale;
		newNode->position.x -= newNode->destRect.w / 2;
		newNode->position.y -= newNode->destRect.h / 2;
		gameObjects.emplace_back(newNode);
	}*/


	temp = EventManager::getInstance().GetEvent(SDL_KEYDOWN);
	tempSize = temp.size();
	if (tempSize > 0)
	{
		for (size_t i = 0; i < tempSize; i++)
		{
			if (temp[i]->key.keysym.sym == SDLK_e)
			{
				int GOSize = gameObjects.size();
				for (size_t i = 0; i < GOSize; i++)
				{
					int GOX = gameObjects[i]->destRect.x;
					int GOY = gameObjects[i]->destRect.y;
					int GOW = gameObjects[i]->destRect.w;
					int GOH = gameObjects[i]->destRect.h;
					if (gameObjects[i]->myType == GOType::NODE && mouseX > GOX && mouseX < GOX + GOW && mouseY > GOY && mouseY < GOY + GOH)//if current obj is a node, and mouse is over the object
					{
						NPC->AddNodeToPath(((GONode*)gameObjects[i]));
					}
				}
			}
		}
	}


}

void GameEngine::Draw()
{
	SDL_RenderClear(renderer);

	_gridFactory->Draw();
	
	std::vector<GameObject*> lateDrawList;

	for (size_t i = 0; i < gameObjects.size(); i++)
	{
		if (gameObjects[i]->myType == GOType::BOID)
		{
			lateDrawList.emplace_back(gameObjects[i]);
		}
		else
		{
			gameObjects[i]->Draw();
		}
	}

	for (size_t i = 0; i < lateDrawList.size(); i++)
	{
		lateDrawList[i]->Draw();
	}



	SDL_SetRenderDrawColor(renderer, backColor.r, backColor.g, backColor.b, backColor.a);
	SDL_RenderPresent(renderer);
}

void GameEngine::Clean()
{
}

void GameEngine::CapFrameRate(int inputFrameRate)
{
	float tempFRControl = (float)(1.0f / inputFrameRate);
	tempFRControl *= 1000;


	bool frameCapped = false;
	if (EventManager::dTime < tempFRControl)
	{
		typedef std::chrono::milliseconds MS;
		typedef std::chrono::seconds SEC;
		typedef std::chrono::seconds NS;
		typedef std::chrono::duration<float> fSec;
		MS time = MS((int)(tempFRControl - EventManager::dTime));
		std::this_thread::sleep_for(time);
		frameCapped = true;
	}

	if (frameCapped)
		std::cout << "FPS: " << 1000 / (EventManager::dTime + (tempFRControl - EventManager::dTime)) << "\n";
	else
		std::cout << "FPS: " << 1000 / (EventManager::dTime) << "\n";
}
