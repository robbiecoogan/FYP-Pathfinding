#include "GridFactory.h"
#include "EventManager.h"

int GridFactory::defaultCellHeight = 10;
int GridFactory::defaultCellWidth = 10;

GridFactory::GridFactory(SDL_Window* _window, SDL_Renderer* _renderer, int _screenWidth, int _screenHeight) 
: window(_window), renderer(_renderer), screenWidth(_screenWidth), screenHeight(_screenHeight), cellWidth(defaultCellWidth), cellHeight(defaultCellHeight)
{
	//find common divisible integer for the screen width and height, and use that as cell width and height
	int currMult = 40;
	while (true)
	{
		float test = (screenWidth / currMult);
		if (test - ((int)test) <= 0)//if there is a remainder, don't bother
		{
			float test = (screenHeight / currMult);
			
			if (test - ((int)test) <= 0)	break;
		}
		currMult++;
	}

	cellWidth = cellHeight = currMult;

}

GridFactory::GridFactory(SDL_Window* _window, SDL_Renderer* _renderer, int _screenWidth, int _screenHeight, int _cellWidth, int _cellHeight) 
: window(_window), renderer(_renderer), screenWidth(_screenWidth), screenHeight(_screenHeight), cellWidth(_cellWidth), cellHeight(_cellHeight){}

GridFactory::~GridFactory()
{

}

void GridFactory::Update()
{
	if (copyGrid.size() > 0)
	{


		static bool leftMouseIsDown = false;
		static bool rightMouseIsDown = false;

		std::vector<SDL_Event*> temp = EventManager::getInstance().GetEvent(SDL_MOUSEBUTTONDOWN);
		int tempSize = temp.size();

		int mouseX, mouseY;
		SDL_GetMouseState(&mouseX, &mouseY);
		mouseX *= EventManager::screenScale;
		mouseY *= EventManager::screenScale;

		int mouseXConv = mouseX / cellWidth;
		int mouseYConv = mouseY / cellHeight;
		int nodesPerRow = screenWidth / cellWidth;
		int currNodeIndex = (nodesPerRow * mouseYConv) + mouseXConv;
		GOGridNode* currNode = copyGrid[currNodeIndex];

		for (size_t i = 0; i < tempSize; i++)
		{
			if (temp[i]->button.button == SDL_BUTTON_LEFT)
				leftMouseIsDown = true;
			else if (temp[i]->button.button == SDL_BUTTON_RIGHT)
				rightMouseIsDown = true;
		}

		temp = EventManager::getInstance().GetEvent(SDL_MOUSEBUTTONUP);
		tempSize = temp.size();
		for (size_t i = 0; i < tempSize; i++)
		{
			if (temp[i]->button.button == SDL_BUTTON_LEFT)
				leftMouseIsDown = false;
			else if (temp[i]->button.button == SDL_BUTTON_RIGHT)
				rightMouseIsDown = false;
		}


		if (leftMouseIsDown)
		{
			SDL_Rect nodeRect = currNode->destRect;
			if (mouseX > nodeRect.x && mouseX < nodeRect.x + nodeRect.w 
				&& mouseY > nodeRect.y && mouseY < nodeRect.y + nodeRect.h)
			{
				currNode->isTraversible = false;
			}
		}
		else if (rightMouseIsDown)
		{
			SDL_Rect nodeRect = currNode->destRect;
			if (mouseX > nodeRect.x && mouseX < nodeRect.x + nodeRect.w
				&& mouseY > nodeRect.y && mouseY < nodeRect.y + nodeRect.h)
			{
				currNode->isTraversible = true;
			}
		}


	}
}

void GridFactory::Draw()
{
	int numObj = copyGrid.size();
	for (size_t i = 0; i < numObj; i++)
	{
		copyGrid[i]->Draw();
	}
}

std::vector<GOGridNode*> GridFactory::CreateGrid()
{
	std::vector<GOGridNode*> returnList;

	int numXCells = screenWidth / cellWidth;
	int numYCells = screenHeight / cellHeight;
	//numXCells++;
	//numYCells++;


	for (size_t y = 0; y < numYCells; y++)
		for (size_t x = 0; x < numXCells; x++)
		{
			Vec2 thisPos = Vec2(cellWidth * x, cellHeight * y);//top left pos of the cell
			GOGridNode* temp = new GOGridNode(window, renderer, thisPos, cellWidth, cellHeight);
			returnList.emplace_back(temp);


		}

	//iterate through again to add the connections
	//add the node connections to the newly created node.
	for (size_t y = 0; y < numYCells; y++)
		for (size_t x = 0; x < numXCells; x++)
		{
			if (y == 0)//if we are on the top of the grid
			{
				int currIndex = (y*numXCells) + x;
				if (x > 0)returnList[currIndex]->AddConnection(returnList[(currIndex - 1)]);							//left node
				if (x > 0)returnList[currIndex]->AddConnection(returnList[(currIndex + numXCells) - 1]);				//bottom-left node
				returnList[currIndex]->AddConnection(returnList[(currIndex + numXCells)]);								//bottom-middle node
				if (x < numXCells - 1)returnList[currIndex]->AddConnection(returnList[(currIndex + numXCells) + 1]);	//bottom-right node
				if (x < numXCells - 1)returnList[currIndex]->AddConnection(returnList[(currIndex + 1)]);				//right node
			}
			else if (y == numYCells - 1)//if we are at the bottom of the grid
			{
				int currIndex = (y*numXCells) + x;
				if (x > 0)returnList[currIndex]->AddConnection(returnList[(currIndex - 1)]);							//left node
				if (x > 0)returnList[currIndex]->AddConnection(returnList[(currIndex - numXCells) - 1]);				//top-left node
				returnList[currIndex]->AddConnection(returnList[(currIndex - numXCells)]);								//top-middle node
				if (x < numXCells - 1)returnList[currIndex]->AddConnection(returnList[(currIndex - numXCells) + 1]);	//top-right node
				if (x < numXCells - 1)returnList[currIndex]->AddConnection(returnList[(currIndex + 1)]);				//right node
			}
			else//if we aren't at the top or bottom of the grid
			{
				int currIndex = (y*numXCells) + x;
				if (x > 0)returnList[currIndex]->AddConnection(returnList[(currIndex - 1)]);							//left node

				if (x > 0)returnList[currIndex]->AddConnection(returnList[(currIndex + numXCells) - 1]);				//bottom-left node
				if (y < numYCells - 1)returnList[currIndex]->AddConnection(returnList[(currIndex + numXCells)]);		//bottom-middle node
				if (x < numXCells - 1)returnList[currIndex]->AddConnection(returnList[(currIndex + numXCells) + 1]);	//bottom-right node

				if (x > 0)returnList[currIndex]->AddConnection(returnList[(currIndex - numXCells) - 1]);				//top-left node
				if (y > 0)returnList[currIndex]->AddConnection(returnList[(currIndex - numXCells)]);					//top-middle node
				if (x < numXCells - 1)returnList[currIndex]->AddConnection(returnList[(currIndex - numXCells) + 1]);	//top-right node

				if (x < numXCells - 1)returnList[currIndex]->AddConnection(returnList[(currIndex + 1)]);				//right node
			}
		}

	copyGrid = returnList;

	return returnList;
}
