#include "GameObject.h"
#include "EventManager.h"

GameObject::GameObject(Vec2 position, SDL_Window * gameWindow, SDL_Renderer * gameRenderer)
{
	window = gameWindow;
	renderer = gameRenderer;


	this->position = Vec2(position.x, position.y);


	destRect.w = 200;
	destRect.h = 200;

	myType = GOType::NA;
}

GameObject::~GameObject()
{
}

void GameObject::Update()
{
	position += velocity;

	//SDL_Rect uses integers for position, meaning that any movement that is less than 1 will not register. xPos and yPos are
	//floats, meaning that they store the true x and y pos of the object, and support moving less than a pixel at a time,
	//this is only displayed in full pixels, however.
	destRect.x = (int)position.x;
	destRect.y = (int)position.y;
}

void GameObject::Draw()
{


}

bool GameObject::SetSprite(std::string fileDir)
{
	if (fileDir != "")
	{
		std::string basePath = SDL_GetBasePath();

		//spriteDir = fileDir.substr(basePath.size(), fileDir.size() - basePath.size());

		fileDir = basePath + fileDir;
		bool success = false;
		SDL_Surface* loadSurface = IMG_Load(fileDir.c_str());

		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		if (loadSurface == NULL) std::cout << "Error Loading Image: " << IMG_GetError() << std::endl;
		else
		{
			success = true;
			SDL_ConvertSurfaceFormat(loadSurface, SDL_PIXELFORMAT_RGBA8888, 0);

			texture = SDL_CreateTextureFromSurface(renderer, loadSurface);

			SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

			sourceRect = loadSurface->clip_rect;
			destRect = sourceRect;
		}

		return success;
	}
	return false;
}


