#include "GameObject.h"
#include "EventManager.h"

GONode::GONode(Vec2 position, SDL_Window* gameWindow, SDL_Renderer* gameRenderer) : GameObject(position, gameWindow, gameRenderer)
{
	myType = GOType::NODE;

	SetSprite("Assets\\circle.png");

	mouseDown = false;
}

void GONode::Update()
{
	int mouseX, mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);
	mouseX *= EventManager::screenScale;
	mouseY *= EventManager::screenScale;

	std::vector<SDL_Event*> temp = EventManager::getInstance().GetEvent(SDL_MOUSEBUTTONDOWN);
	int tempSize = temp.size();

	for (size_t i = 0; i < tempSize; i++)
	{
		if (mouseX > position.x && mouseX < position.x + destRect.w && mouseY > position.y && mouseY < position.y + destRect.h && temp[i]->button.button == SDL_BUTTON_LEFT)
			mouseDown = true;
	}
	
	temp = EventManager::getInstance().GetEvent(SDL_MOUSEBUTTONUP);
	tempSize = temp.size();


	if (tempSize > 0)
	{
		mouseDown = false;
	}
		

	if (mouseDown)
		position = Vec2(mouseX - destRect.w / 2, mouseY - destRect.h / 2);


	position += velocity;

	//SDL_Rect uses integers for position, meaning that any movement that is less than 1 will not register. xPos and yPos are
	//floats, meaning that they store the true x and y pos of the object, and support moving less than a pixel at a time,
	//this is only displayed in full pixels, however.
	destRect.x = (int)position.x;
	destRect.y = (int)position.y;
}

void GONode::AddConnection(GONode * inputNode)
{
	bool newNode = true;
	int connectedCount = connectedNodes.size();
	//need to ensure the node is not already on the list.
	for (size_t i = 0; i < connectedCount; i++)
	{
		if (inputNode == connectedNodes[i])
		{
			newNode = false;
			break;
		}
	}

	if (newNode)
		connectedNodes.emplace_back(inputNode);//add the other node to this node's list.
	else
		std::cout << "Cannot add same node twice.\n";
}

void GONode::Draw()
{
	SDL_RenderCopy(renderer, texture, &sourceRect, &destRect);
	for (size_t i = 0; i < connectedNodes.size(); i++)
	{
		
		Vec2 conNode(connectedNodes[i]->position.x + connectedNodes[i]->destRect.w/2, connectedNodes[i]->position.y + connectedNodes[i]->destRect.h/2);
		Vec2 thisNode(position.x + destRect.w/2, position.y + destRect.h/2);
		Vec2 diffVec(conNode.x - thisNode.x, conNode.y - thisNode.y);
		//SDL_RenderDrawLine(renderer, position.x + destRect.w/2, position.y + destRect.h/2, connectedNodes[i]->position.x + connectedNodes[i]->destRect.w/2, connectedNodes[i]->position.y + connectedNodes[i]->destRect.h/2);

		int lineOffset = 5 * EventManager::screenScale;

		SDL_SetRenderDrawColor(renderer, 0, 150, 0, 255);
		SDL_RenderDrawLine(renderer, thisNode.x- lineOffset, thisNode.y- lineOffset, thisNode.x + (diffVec.x/2), thisNode.y + (diffVec.y/2));
		SDL_SetRenderDrawColor(renderer, 150, 0, 0, 255);
		SDL_RenderDrawLine(renderer, conNode.x+ lineOffset, conNode.y+ lineOffset, conNode.x - (diffVec.x / 2), conNode.y - (diffVec.y / 2));
	}
}