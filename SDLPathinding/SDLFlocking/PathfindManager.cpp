#include "PathfindManager.h"

#include <chrono>
#include <thread>

PathfindManager::PathfindManager(SDL_Window* _window, SDL_Renderer* _renderer, int _screenWidth, int _screenHeight, int _cellWidth, int _cellHeight)
{
	window = _window;
	renderer = _renderer;
	screenWidth = _screenWidth;
	screenHeight = _screenHeight;
	cellWidth = _cellWidth;
	cellHeight = _cellHeight;
}

PathfindManager::~PathfindManager()
{
}

///Pathfinding algorithms////////////////////////////////////////////////////
std::vector<Vec2> PathfindManager::AStarPathfind(Vec2 startPos, Vec2 goalPos, std::vector<GOGridNode*> searchGrid)
{
	searchGridCopy = searchGrid;
	std::vector<Vec2> returnList;

	//find closest node to startPos
	int xAlong = startPos.x / cellWidth;
	int yAlong = startPos.y / cellHeight;
	int yCells = screenWidth / cellWidth;//how many cells are there per row.
	int cellIndex = (yCells * yAlong) + xAlong;

	if (cellIndex > searchGrid.size() - 1) cellIndex = searchGrid.size() - 1;
	if (cellIndex < 0) cellIndex = 0;

	GOGridNode* currNode = searchGrid[cellIndex];


	xAlong = goalPos.x / cellWidth;
	yAlong = goalPos.y / cellHeight;
	cellIndex = (yCells * yAlong) + xAlong;

	GOGridNode* targetNode = searchGrid[cellIndex];
	if (targetNode->isTraversible == false) return returnList;


	std::vector<GOGridNode*> openList;
	std::vector<GOGridNode*> closedList;

	openList.emplace_back(currNode);
	bool noSolution = false;
	
	while (openList.size() > 0)
	{
		currNode = openList[0];
		currNode->gCost = GetGCost(currNode);
		currNode->hCost = ManhattanDist(currNode, targetNode);;
		int openSize = openList.size();



		int lowestIndex = 0;
		for (size_t i = 1; i < openSize; i++)
		{
			float gCost, hCost;
			gCost = GetGCost(openList[i]);// gCost is distance of node to start pos.
			hCost = ManhattanDist(openList[i], targetNode);// hcost is manhattan distance of curr node to end node
			//hCost = EuclideanDist(openList[i], targetNode);
			float fCost = gCost + hCost;

			openList[i]->gCost = gCost;
			openList[i]->hCost = hCost;

			if (fCost <= currNode->GetFCost() && hCost < currNode->hCost) currNode = openList[i], lowestIndex = i;
		}
		openList.erase(openList.begin() + lowestIndex);//removed current node from open list
		closedList.emplace_back(currNode);

		if (currNode == targetNode)break;//if we have found the end node, the path has been found

		//for each neighbour of the current node, if the neighbour is not traversable, or is in the closed list, skip the node
		std::vector<GOGridNode*> connectedNodes = currNode->GridGetConnectedNodes();
		int neighbourCount = connectedNodes.size();

		GOGridNode* cheapestNeighbour = nullptr;
		if (neighbourCount > 0) cheapestNeighbour = connectedNodes[0];
		for (size_t i = 0; i < neighbourCount; i++)
		{
			GOGridNode* currNeighbour = connectedNodes[i];


			/////////////////////////////
			if (!connectedNodes[i]->isTraversible) continue;//if not traversible, continue to check next element
			int closedSize = closedList.size();
			bool skipToNext = false;
			for (size_t j = 0; j < closedSize; j++)//if is in the closed list, continue to next element
				if (closedList[j] == currNeighbour) skipToNext = true;

			if (skipToNext) continue;


			//get fCost
			float gCost, hCost;
			gCost = GetGCost(currNeighbour);// gCost is distance of node to start pos.
			hCost = ManhattanDist(currNeighbour, targetNode);// hcost is distance of curr node to end node

			currNeighbour->gCost = gCost;
			currNeighbour->hCost = hCost;

			float fCost = gCost + hCost;

			if (currNeighbour == currNode->GetTLNode() || currNeighbour == currNode->GetTRNode())
			{
				if ((currNode->GetTopNode() && !currNode->GetTopNode()->isTraversible) 
					|| (currNode->GetLeftNode() && !currNode->GetLeftNode()->isTraversible) 
					|| (currNode->GetRightNode() && !currNode->GetRightNode()->isTraversible))
						continue;
			}
			else if (currNeighbour == currNode->GetBLNode() || currNeighbour == currNode->GetBRNode())
			{
				if ((currNode->GetBottomNode() && !currNode->GetBottomNode()->isTraversible)
					|| (currNode->GetLeftNode() && !currNode->GetLeftNode()->isTraversible) 
					|| (currNode->GetRightNode() && !currNode->GetRightNode()->isTraversible))
						continue;
			}

			//if not on open list, add it to open list.
			openSize = openList.size();
			bool foundItem = false;
			for (size_t j = 0; j < openSize; j++)
			{
				if (currNeighbour == openList[j]) foundItem = true;
			}

			int stepCost = currNode->gCost + EuclideanDist(currNode, currNeighbour);
			if (stepCost < currNeighbour->gCost || !foundItem)
			{
				currNeighbour->gCost = stepCost;
				currNeighbour->hCost = EuclideanDist(currNeighbour, targetNode);
				currNeighbour->parent = currNode;

				if (!foundItem)
				{
					openList.emplace_back(currNeighbour);
				}
			}

		}

		DrawPathfindLists(&openList, &closedList, &startPos, &goalPos);
	}
	if (openList.size() == 0 && currNode != targetNode) noSolution = true;//if the open list is empty, theres nothing left to explore. No path can be found.
	//once this has been done, we should be able to keep traversing the end node parent pointer until it is a nullptr to find the path.
	currNode = targetNode;
	while (currNode != nullptr && !noSolution)
	{
		Vec2 newPos = currNode->GetNodePos();
		returnList.emplace_back(newPos);

		GOGridNode* temp = currNode;
		currNode = currNode->parent;
		temp->parent = nullptr;
		returnList[0] = goalPos;
	}

	std::reverse(returnList.begin(), returnList.end());

	return returnList;
}

std::vector<Vec2> PathfindManager::JPSPathfind(Vec2 startPos, Vec2 goalPos, std::vector<GOGridNode*> searchGrid)
{
	std::vector<Vec2> returnList;
	searchGridCopy = searchGrid;
	//SDL_RenderClear(renderer);

	//find closest node to startPos
	int xAlong = startPos.x / cellWidth;
	int yAlong = startPos.y / cellHeight;
	int yCells = screenWidth / cellWidth;//how many cells are there per row.
	int cellIndex = (yCells * yAlong) + xAlong;

	if (cellIndex > searchGrid.size() - 1) cellIndex = searchGrid.size() - 1;
	if (cellIndex < 0) cellIndex = 0;

	GOGridNode* startNode = searchGrid[cellIndex];


	xAlong = goalPos.x / cellWidth;
	yAlong = goalPos.y / cellHeight;
	cellIndex = (yCells * yAlong) + xAlong;

	GOGridNode* endNode = searchGrid[cellIndex];
	if (endNode->isTraversible == false) return returnList;

	endNode->parent = nullptr;

	//Anything on the openlist is a JPSNode, which has a pointer to a gridnode (the current node) and a direction (the direction it will travel)
	//1. a while loop should iterate until the openlist is empty
	//start the openlist with all directions added, as this is the one stage that we don't know which direction to go in.

	std::vector<JPSNode*> openList;
	std::vector<JPSNode*> closedList;

	JPSNode* up = new JPSNode(startNode, DIR_UP); openList.emplace_back(up), up->node->parent = startNode;
	JPSNode* right = new JPSNode(startNode, DIR_RIGHT); openList.emplace_back(right), right->node->parent = startNode;
	JPSNode* down = new JPSNode(startNode, DIR_DOWN); openList.emplace_back(down), down->node->parent = startNode;
	JPSNode* left = new JPSNode(startNode, DIR_LEFT); openList.emplace_back(left), left->node->parent = startNode;
	JPSNode* topLeft = new JPSNode(startNode, DIR_TOPLEFT); openList.emplace_back(topLeft), topLeft->node->parent = startNode;
	JPSNode* topRight = new JPSNode(startNode, DIR_TOPRIGHT); openList.emplace_back(topRight), topRight->node->parent = startNode;
	JPSNode* bottomLeft = new JPSNode(startNode, DIR_BOTTOMLEFT); openList.emplace_back(bottomLeft), bottomLeft->node->parent = startNode;
	JPSNode* bottomRight = new JPSNode(startNode, DIR_BOTTOMRIGHT); openList.emplace_back(bottomRight), bottomRight->node->parent = startNode;

	int openSize = openList.size();

	//lambda for adding a jump point, but searches if it is in the closed list before doing so
	auto AddToOpen = [&](JPSNode* addNode)
	{
		int closedSize = closedList.size();
		bool foundCopy = false;
		for (size_t i = 0; i < closedSize; i++)
		{
			if (addNode->node == closedList[i]->node && addNode->dir == closedList[i]->dir)//if already in list
			{
				foundCopy = true;
				break;
			}
		}
		if (!foundCopy)
		{
			openList.emplace_back(addNode);
		}
	};

	auto IsOnClosedList = [&closedList](JPSNode* _input)
	{
		int closedSize = closedList.size();
		for (size_t i = 0; i < closedSize; i++)
		{
			if (_input->node == closedList[i]->node)
			{
				return true;
			}
		}
		return false;
	};


	//lambdas for iterating in different directions
	SDL_Renderer* renderCopy = renderer;
	auto iterateUp = [&openList, &closedList, &renderCopy, &AddToOpen, &IsOnClosedList](JPSNode* myStartNode, GOGridNode* _endNode)
	{
		GOGridNode* tempCurr = myStartNode->node;
		if (tempCurr->GetTopNode() && tempCurr->GetTopNode()->isTraversible && !IsOnClosedList(&JPSNode(tempCurr->GetTopNode(), DIR_NONE)))
		{
			tempCurr = tempCurr->GetTopNode();
			SDL_SetRenderDrawColor(renderCopy, 0, 100, 0, 255);
			SDL_RenderFillRect(renderCopy, &tempCurr->destRect);
			SDL_SetRenderDrawColor(renderCopy, 0, 0, 0, 255);
			SDL_RenderPresent(renderCopy);
		}
		while (tempCurr->GetTopNode() && tempCurr->GetTopNode()->isTraversible && !IsOnClosedList(&JPSNode(tempCurr, DIR_NONE)))//if there is a node above us, move up
		{
			SDL_SetRenderDrawColor(renderCopy, 0, 100, 0, 255);
			SDL_RenderFillRect(renderCopy, &tempCurr->destRect);
			SDL_SetRenderDrawColor(renderCopy, 0, 0, 0, 255);
			SDL_RenderPresent(renderCopy);
			bool addedNodes = false;
			//first, check if there is anything next to our node, as we will need to add things to the open list if this is the case.
			if (tempCurr->GetLeftNode() && !tempCurr->GetLeftNode()->isTraversible)
			{
				//if diagonally in front of us is another intraversible wall, there is no point adding to the open list yet, as we will do that at the end of the wall.
				if (tempCurr->GetTLNode() && tempCurr->GetTLNode()->isTraversible && !IsOnClosedList(&JPSNode(tempCurr->GetTLNode(), DIR_NONE)))
				{
					//add 2 new nodes that will search top left, and will continue searching up afterwards
					JPSNode* temp = new JPSNode(tempCurr, DIR_TOPLEFT);
					temp->node->parent = myStartNode->node;
					AddToOpen(temp);
					closedList.emplace_back(temp);
					addedNodes = true;
				}
			}
			if (tempCurr->GetRightNode() && !tempCurr->GetRightNode()->isTraversible)
			{
				//if diagonally in front of us is another intraversible wall, there is no point adding to the open list yet, as we will do that at the end of the wall.
				if (tempCurr->GetTRNode() && tempCurr->GetTRNode()->isTraversible && !IsOnClosedList(&JPSNode(tempCurr->GetTRNode(), DIR_NONE)))
				{
					//add 2 new nodes that will search top left, and will continue searching up afterwards
					JPSNode* temp = new JPSNode(tempCurr, DIR_TOPRIGHT);
					temp->node->parent = myStartNode->node;
					AddToOpen(temp);
					closedList.emplace_back(temp);
					addedNodes = true;
				}
			}

			if (tempCurr == _endNode || (tempCurr->GetTLNode() == _endNode) || (tempCurr->GetTRNode() == _endNode))
			{
				JPSNode* goalTemp = new JPSNode(_endNode, DIR_NONE);
				goalTemp->node->parent = myStartNode->node;
				openList.emplace_back(goalTemp);
				return true;
			}


			if (!addedNodes)
			{
				tempCurr = tempCurr->GetTopNode();
			}
			else
			{
				JPSNode* temp = new JPSNode(tempCurr, DIR_UP);
				temp->node->parent = myStartNode->node;
				AddToOpen(temp);
				closedList.emplace_back(temp);
				return true;
			}

		}
		
		if (tempCurr == _endNode || (tempCurr->GetTLNode() == _endNode) || (tempCurr->GetTRNode() == _endNode))
		{
			JPSNode* goalTemp = new JPSNode(_endNode, DIR_NONE);
			goalTemp->node->parent = myStartNode->node;
			openList.emplace_back(goalTemp);
			return true;
		}

		return false;
	};

	auto iterateRight = [&openList, &closedList, &renderCopy, &AddToOpen, &IsOnClosedList](JPSNode* myStartNode, GOGridNode* _endNode)
	{
		GOGridNode* tempCurr = myStartNode->node;
		if (tempCurr->GetRightNode() && tempCurr->GetRightNode()->isTraversible && !IsOnClosedList(&JPSNode(tempCurr->GetRightNode(), DIR_NONE)))
		{
			tempCurr = tempCurr->GetRightNode();
			SDL_SetRenderDrawColor(renderCopy, 0, 100, 0, 255);
			SDL_RenderFillRect(renderCopy, &tempCurr->destRect);
			SDL_SetRenderDrawColor(renderCopy, 0, 0, 0, 255);
			SDL_RenderPresent(renderCopy);
		}
		while (tempCurr->GetRightNode() && tempCurr->GetRightNode()->isTraversible && !IsOnClosedList(&JPSNode(tempCurr->GetRightNode(), DIR_NONE)))
		{
			SDL_SetRenderDrawColor(renderCopy, 100, 0, 0, 255);
			SDL_RenderFillRect(renderCopy, &tempCurr->destRect);
			SDL_SetRenderDrawColor(renderCopy, 0, 0, 0, 255);
			SDL_RenderPresent(renderCopy);
			bool addedNodes = false;
			//first, check if there is anything next to our node, as we will need to add things to the open list if this is the case.
			if (tempCurr->GetTopNode() && !tempCurr->GetTopNode()->isTraversible)
			{
				//if diagonally in front of us is another intraversible wall, there is no point adding to the open list yet, as we will do that at the end of the wall.
				if (tempCurr->GetTRNode() && tempCurr->GetTRNode()->isTraversible && !IsOnClosedList(&JPSNode(tempCurr->GetTRNode(), DIR_NONE)))
				{
					//add 2 new nodes that will search top left, and will continue searching up afterwards
					JPSNode* temp = new JPSNode(tempCurr, DIR_TOPRIGHT);
					temp->node->parent = myStartNode->node;
					AddToOpen(temp);
					closedList.emplace_back(temp);
					addedNodes = true;
				}
			}
			if (tempCurr->GetBottomNode() && !tempCurr->GetBottomNode()->isTraversible)
			{
				//if diagonally in front of us is another intraversible wall, there is no point adding to the open list yet, as we will do that at the end of the wall.
				if (tempCurr->GetBRNode() && tempCurr->GetBRNode()->isTraversible && !IsOnClosedList(&JPSNode(tempCurr->GetBRNode(), DIR_NONE)))
				{
					//add 2 new nodes that will search top left, and will continue searching up afterwards
					JPSNode* temp = new JPSNode(tempCurr, DIR_BOTTOMRIGHT);
					temp->node->parent = myStartNode->node;
					AddToOpen(temp);
					closedList.emplace_back(temp);
					addedNodes = true;
				}
			}


			if (tempCurr == _endNode || (tempCurr->GetTopNode() == _endNode) || (tempCurr->GetBottomNode() == _endNode))
			{
				JPSNode* goalTemp = new JPSNode(_endNode, DIR_NONE);
				goalTemp->node->parent = myStartNode->node;
				openList.emplace_back(goalTemp);
				return true;
			}


			if (!addedNodes)
			{
				tempCurr = tempCurr->GetRightNode();
			}
			else
			{
				JPSNode* temp = new JPSNode(tempCurr, DIR_RIGHT);
				temp->node->parent = myStartNode->node;
				AddToOpen(temp);
				closedList.emplace_back(temp);
				return true;
			}

		}
		
		if (tempCurr == _endNode || (tempCurr->GetTopNode() == _endNode) || (tempCurr->GetBottomNode() == _endNode))
		{
			JPSNode* goalTemp = new JPSNode(_endNode, DIR_NONE);
			goalTemp->node->parent = myStartNode->node;
			openList.emplace_back(goalTemp);
			return true;
		}

		return false;
	};
	auto iterateDown = [&openList, &closedList, &renderCopy, &AddToOpen, &IsOnClosedList](JPSNode* myStartNode, GOGridNode* _endNode)
	{
		GOGridNode* tempCurr = myStartNode->node;
		if (tempCurr->GetBottomNode() && tempCurr->GetBottomNode()->isTraversible && !IsOnClosedList(&JPSNode(tempCurr->GetBottomNode(), DIR_NONE)))
		{
			tempCurr = tempCurr->GetBottomNode();
			SDL_SetRenderDrawColor(renderCopy, 0, 100, 0, 255);
			SDL_RenderFillRect(renderCopy, &tempCurr->destRect);
			SDL_SetRenderDrawColor(renderCopy, 0, 0, 0, 255);
			SDL_RenderPresent(renderCopy);
		}
		while (tempCurr->GetBottomNode() && tempCurr->GetBottomNode()->isTraversible && !IsOnClosedList(&JPSNode(tempCurr->GetBottomNode(), DIR_NONE)))
		{

			bool addedNodes = false;
			//first, check if there is anything next to our node, as we will need to add things to the open list if this is the case.
			if (tempCurr->GetLeftNode() && !tempCurr->GetLeftNode()->isTraversible)
			{
				//if diagonally in front of us is another intraversible wall, there is no point adding to the open list yet, as we will do that at the end of the wall.
				if (tempCurr->GetBLNode() && tempCurr->GetBLNode()->isTraversible && !IsOnClosedList(&JPSNode(tempCurr->GetBLNode(), DIR_NONE)))
				{
					//add 2 new nodes that will search top left, and will continue searching up afterwards
					JPSNode* temp = new JPSNode(tempCurr, DIR_BOTTOMLEFT);
					temp->node->parent = myStartNode->node;
					AddToOpen(temp);
					closedList.emplace_back(temp);
					addedNodes = true;
				}
			}
			if (tempCurr->GetRightNode() && !tempCurr->GetRightNode()->isTraversible)
			{
				//if diagonally in front of us is another intraversible wall, there is no point adding to the open list yet, as we will do that at the end of the wall.
				if (tempCurr->GetBRNode() && tempCurr->GetBRNode()->isTraversible && !IsOnClosedList(&JPSNode(tempCurr->GetBRNode(), DIR_NONE)))
				{
					//add 2 new nodes that will search top left, and will continue searching up afterwards
					JPSNode* temp = new JPSNode(tempCurr, DIR_BOTTOMRIGHT);
					temp->node->parent = myStartNode->node;
					AddToOpen(temp);
					closedList.emplace_back(temp);
					addedNodes = true;
				}
			}


			if (tempCurr == _endNode || (tempCurr->GetBLNode() == _endNode) || (tempCurr->GetRightNode() == _endNode))
			{
				JPSNode* goalTemp = new JPSNode(_endNode, DIR_NONE);
				goalTemp->node->parent = myStartNode->node;
				openList.emplace_back(goalTemp);
				return true;
			}


			if (!addedNodes)
			{
				tempCurr = tempCurr->GetBottomNode();
			}
			else
			{
				JPSNode* temp = new JPSNode(tempCurr, DIR_DOWN);
				temp->node->parent = myStartNode->node;
				AddToOpen(temp);
				closedList.emplace_back(temp);
				return true;
			}
			SDL_SetRenderDrawColor(renderCopy, 0, 100, 0, 255);
			SDL_RenderFillRect(renderCopy, &tempCurr->destRect);
			SDL_SetRenderDrawColor(renderCopy, 0, 0, 0, 255);
			SDL_RenderPresent(renderCopy);

		}

		if (tempCurr == _endNode || (tempCurr->GetBLNode() == _endNode) || (tempCurr->GetRightNode() == _endNode))
		{
			JPSNode* goalTemp = new JPSNode(_endNode, DIR_NONE);
			goalTemp->node->parent = myStartNode->node;
			openList.emplace_back(goalTemp);
			return true;
		}

		return false;
	};
	auto iterateLeft = [&openList, &closedList, &renderCopy, &AddToOpen, &IsOnClosedList](JPSNode* myStartNode, GOGridNode* _endNode)
	{
		GOGridNode* tempCurr = myStartNode->node;
		if (tempCurr->GetLeftNode() && tempCurr->GetLeftNode()->isTraversible && !IsOnClosedList(&JPSNode(tempCurr->GetLeftNode(), DIR_NONE)))
		{
			tempCurr = tempCurr->GetLeftNode();
			SDL_SetRenderDrawColor(renderCopy, 0, 100, 0, 255);
			SDL_RenderFillRect(renderCopy, &tempCurr->destRect);
			SDL_SetRenderDrawColor(renderCopy, 0, 0, 0, 255);
			SDL_RenderPresent(renderCopy);
		}
		while (tempCurr->GetLeftNode() && tempCurr->GetLeftNode()->isTraversible && !IsOnClosedList(&JPSNode(tempCurr->GetLeftNode(), DIR_NONE)))
		{
			SDL_SetRenderDrawColor(renderCopy, 100, 0, 0, 255);
			SDL_RenderFillRect(renderCopy, &tempCurr->destRect);
			SDL_SetRenderDrawColor(renderCopy, 0, 0, 0, 255);
			SDL_RenderPresent(renderCopy);
			bool addedNodes = false;
			//first, check if there is anything next to our node, as we will need to add things to the open list if this is the case.
			if (tempCurr->GetTopNode() && !tempCurr->GetTopNode()->isTraversible)
			{
				//if diagonally in front of us is another intraversible wall, there is no point adding to the open list yet, as we will do that at the end of the wall.
				if (tempCurr->GetTLNode() && tempCurr->GetTLNode()->isTraversible && !IsOnClosedList(&JPSNode(tempCurr->GetTLNode(), DIR_NONE)))
				{
					//add 2 new nodes that will search top left, and will continue searching up afterwards
					JPSNode* temp = new JPSNode(tempCurr, DIR_TOPLEFT);
					temp->node->parent = myStartNode->node;
					AddToOpen(temp);
					closedList.emplace_back(temp);
					addedNodes = true;
				}
			}
			if (tempCurr->GetBottomNode() && !tempCurr->GetBottomNode()->isTraversible)
			{
				//if diagonally in front of us is another intraversible wall, there is no point adding to the open list yet, as we will do that at the end of the wall.
				if (tempCurr->GetBLNode() && tempCurr->GetBLNode()->isTraversible && !IsOnClosedList(&JPSNode(tempCurr->GetBLNode(), DIR_NONE)))
				{
					//add 2 new nodes that will search top left, and will continue searching up afterwards
					JPSNode* temp = new JPSNode(tempCurr, DIR_BOTTOMLEFT);
					temp->node->parent = myStartNode->node;
					AddToOpen(temp);
					closedList.emplace_back(temp);
					addedNodes = true;
				}
			}


			if (tempCurr == _endNode || (tempCurr->GetTLNode() == _endNode) || (tempCurr->GetBottomNode() == _endNode))
			{
				JPSNode* goalTemp = new JPSNode(_endNode, DIR_NONE);
				goalTemp->node->parent = myStartNode->node;
				openList.emplace_back(goalTemp);
				return true;
			}


			if (!addedNodes)
			{
				tempCurr = tempCurr->GetLeftNode();
			}
			else
			{
				JPSNode* temp = new JPSNode(tempCurr, DIR_LEFT);
				temp->node->parent = myStartNode->node;
				AddToOpen(temp);
				closedList.emplace_back(temp);
				return true;
			}

		}
		
		if (tempCurr == _endNode || (tempCurr->GetTLNode() == _endNode) || (tempCurr->GetBottomNode() == _endNode))
		{
			JPSNode* goalTemp = new JPSNode(_endNode, DIR_NONE);
			goalTemp->node->parent = myStartNode->node;
			openList.emplace_back(goalTemp);
			return true;
		}

		return false;
	};
	auto iterateTL = [&openList, &closedList, &renderCopy, &iterateLeft, &iterateUp, &AddToOpen, &IsOnClosedList](JPSNode* myStartNode, GOGridNode* _endNode)
	{
		GOGridNode* tempCurr = myStartNode->node;
		if (tempCurr->GetTLNode() && tempCurr->GetTLNode()->isTraversible && !IsOnClosedList(&JPSNode(tempCurr->GetTLNode(), DIR_NONE)))
		{
			tempCurr = tempCurr->GetTLNode();
		}
		while (tempCurr && tempCurr->isTraversible && !IsOnClosedList(&JPSNode(tempCurr, DIR_NONE)))
		{

			bool addedNodes = false;

			tempCurr->parent = myStartNode->node;
			JPSNode* tempNode = new JPSNode(tempCurr, DIR_NONE);
			if (iterateLeft(tempNode, _endNode))
			{
				JPSNode* temp = new JPSNode(tempCurr, DIR_TOPLEFT);
				temp->node->parent = myStartNode->node;
				AddToOpen(temp);
				closedList.emplace_back(temp);
				addedNodes = true;
			}
			delete tempNode;
			tempNode = new JPSNode(tempCurr, DIR_NONE);
			if (iterateUp(tempNode, _endNode)) 
			{
				JPSNode* temp = new JPSNode(tempCurr, DIR_TOPLEFT);
				temp->node->parent = myStartNode->node;
				AddToOpen(temp);
				closedList.emplace_back(temp);
				addedNodes = true;
			}
			delete tempNode;
			SDL_SetRenderDrawColor(renderCopy, 255, 255, 0, 255);
			SDL_RenderFillRect(renderCopy, &tempCurr->destRect);
			SDL_SetRenderDrawColor(renderCopy, 0, 0, 0, 255);
			SDL_RenderPresent(renderCopy);

			//check below and to our side, as this is a diagonal check
			bool firstCheckTrue = false;
			if (tempCurr->GetBottomNode() && !tempCurr->GetBottomNode()->isTraversible)
			{

				if (tempCurr->GetBLNode() && tempCurr->GetBLNode()->isTraversible && !IsOnClosedList(&JPSNode(tempCurr->GetBLNode(), DIR_NONE)))
				{

					JPSNode* temp = new JPSNode(tempCurr, DIR_BOTTOMLEFT);
					temp->node->parent = myStartNode->node;
					AddToOpen(temp);
					closedList.emplace_back(temp);
					addedNodes = true;
				}
				firstCheckTrue = true;
			}
			if (tempCurr->GetRightNode() && !tempCurr->GetRightNode()->isTraversible)
			{

				if (tempCurr->GetTRNode() && tempCurr->GetTRNode()->isTraversible && !IsOnClosedList(&JPSNode(tempCurr->GetTRNode(), DIR_NONE)))
				{

					JPSNode* temp = new JPSNode(tempCurr, DIR_TOPRIGHT);
					temp->node->parent = myStartNode->node;
					AddToOpen(temp);
					closedList.emplace_back(temp);
					addedNodes = true;
				}
				if (firstCheckTrue)
				{
					tempCurr->parent = nullptr;
					break;
				}
			}


			if (tempCurr == _endNode || (tempCurr->GetBLNode() == _endNode) || (tempCurr->GetTRNode() == _endNode))
			{
				JPSNode* goalTemp = new JPSNode(_endNode, DIR_NONE);
				goalTemp->node->parent = myStartNode->node;
				openList.emplace_back(goalTemp);
				break;
			}

			tempCurr = tempCurr->GetTLNode();
			//if (!addedNodes)
			//{
			//	tempCurr = tempCurr->GetTLNode();
			//}
			//else
			//{
			//	JPSNode* temp = new JPSNode(tempCurr, DIR_TOPLEFT);
			//	temp->node->parent = myStartNode->node;
			//	AddToOpen(temp);
			//	closedList.emplace_back(temp);
			//	break;
			//}

		}
	};
	auto iterateTR = [&openList, &closedList, &renderCopy, &iterateRight, &iterateUp, &AddToOpen, &IsOnClosedList](JPSNode* myStartNode, GOGridNode* _endNode)
	{
		GOGridNode* tempCurr = myStartNode->node;
		if (tempCurr->GetTRNode() && tempCurr->GetTRNode()->isTraversible && !IsOnClosedList(&JPSNode(tempCurr->GetTRNode(), DIR_NONE)))
		{
			tempCurr = tempCurr->GetTRNode();
		}
		while (tempCurr && tempCurr->isTraversible && !IsOnClosedList(&JPSNode(tempCurr, DIR_NONE)))
		{

			bool addedNodes = false;

			tempCurr->parent = myStartNode->node;
			JPSNode* tempNode = new JPSNode(tempCurr, DIR_NONE);
			if (iterateRight(tempNode, _endNode))
			{
				JPSNode* temp = new JPSNode(tempCurr, DIR_TOPLEFT);
				temp->node->parent = myStartNode->node;
				AddToOpen(temp);
				closedList.emplace_back(temp);
				addedNodes = true;
			}
			delete tempNode;
			tempNode = new JPSNode(tempCurr, DIR_NONE);
			if (iterateUp(tempNode, _endNode))
			{
				JPSNode* temp = new JPSNode(tempCurr, DIR_TOPLEFT);
				temp->node->parent = myStartNode->node;
				AddToOpen(temp);
				closedList.emplace_back(temp);
				addedNodes = true;
			}
			delete tempNode;
			SDL_SetRenderDrawColor(renderCopy, 255, 255, 0, 255);
			SDL_RenderFillRect(renderCopy, &tempCurr->destRect);
			SDL_SetRenderDrawColor(renderCopy, 0, 0, 0, 255);
			SDL_RenderPresent(renderCopy);

			//check below and to our side, as this is a diagonal check
			bool firstCheckTrue = false;
			if (tempCurr->GetBottomNode() && !tempCurr->GetBottomNode()->isTraversible)
			{

				if (tempCurr->GetBRNode() && tempCurr->GetBRNode()->isTraversible && !IsOnClosedList(&JPSNode(tempCurr->GetBRNode(), DIR_NONE)))
				{

					JPSNode* temp = new JPSNode(tempCurr, DIR_BOTTOMRIGHT);
					temp->node->parent = myStartNode->node;
					AddToOpen(temp);
					closedList.emplace_back(temp);
					addedNodes = true;
				}
				firstCheckTrue = true;
			}
			if (tempCurr->GetLeftNode() && !tempCurr->GetLeftNode()->isTraversible)
			{

				if (tempCurr->GetTLNode() && tempCurr->GetTLNode()->isTraversible && !IsOnClosedList(&JPSNode(tempCurr->GetTLNode(), DIR_NONE)))
				{

					JPSNode* temp = new JPSNode(tempCurr, DIR_TOPLEFT);
					temp->node->parent = myStartNode->node;
					AddToOpen(temp);
					closedList.emplace_back(temp);
					addedNodes = true;
				}
				if (firstCheckTrue)
				{
					tempCurr->parent = nullptr;
					break;
				}
			}


			if (tempCurr == _endNode || (tempCurr->GetBRNode() == _endNode) || (tempCurr->GetTLNode() == _endNode))
			{
				JPSNode* goalTemp = new JPSNode(_endNode, DIR_NONE);
				goalTemp->node->parent = myStartNode->node;
				openList.emplace_back(goalTemp);
				break;
			}

			tempCurr = tempCurr->GetTRNode();
			//if (!addedNodes)
			//{
			//	tempCurr = tempCurr->GetTRNode();
			//}
			//else
			//{
			//	JPSNode* temp = new JPSNode(tempCurr, DIR_TOPRIGHT);
			//	temp->node->parent = myStartNode->node;
			//	AddToOpen(temp);
			//	closedList.emplace_back(temp);
			//	break;
			//}

		}
	};
	auto iterateBL = [&openList, &closedList, &renderCopy, &iterateLeft, &iterateDown, &AddToOpen, &IsOnClosedList](JPSNode* myStartNode, GOGridNode* _endNode)
	{
		GOGridNode* tempCurr = myStartNode->node;
		if (tempCurr->GetBLNode() && tempCurr->GetBLNode()->isTraversible && !IsOnClosedList(&JPSNode(tempCurr->GetBLNode(), DIR_NONE)))
		{
			tempCurr = tempCurr->GetBLNode();
		}
		while (tempCurr && tempCurr->isTraversible && !IsOnClosedList(&JPSNode(tempCurr, DIR_NONE)))
		{

			bool addedNodes = false;

			tempCurr->parent = myStartNode->node;
			JPSNode* tempNode = new JPSNode(tempCurr, DIR_NONE);
			if (iterateLeft(tempNode, _endNode))
			{
				JPSNode* temp = new JPSNode(tempCurr, DIR_TOPLEFT);
				temp->node->parent = myStartNode->node;
				AddToOpen(temp);
				closedList.emplace_back(temp);
				addedNodes = true;
			}
			delete tempNode;
			tempNode = new JPSNode(tempCurr, DIR_NONE);
			if (iterateDown(tempNode, _endNode))
			{
				JPSNode* temp = new JPSNode(tempCurr, DIR_TOPLEFT);
				temp->node->parent = myStartNode->node;
				AddToOpen(temp);
				closedList.emplace_back(temp);
				addedNodes = true;
			}
			delete tempNode;
			SDL_SetRenderDrawColor(renderCopy, 255, 255, 0, 255);
			SDL_RenderFillRect(renderCopy, &tempCurr->destRect);
			SDL_SetRenderDrawColor(renderCopy, 0, 0, 0, 255);
			SDL_RenderPresent(renderCopy);

			//check below and to our side, as this is a diagonal check
			bool firstCheckTrue = false;
			if (tempCurr->GetTopNode() && !tempCurr->GetTopNode()->isTraversible)
			{

				if (tempCurr->GetTLNode() && tempCurr->GetTLNode()->isTraversible && !IsOnClosedList(&JPSNode(tempCurr->GetTLNode(), DIR_NONE)))
				{

					JPSNode* temp = new JPSNode(tempCurr, DIR_TOPLEFT);
					temp->node->parent = myStartNode->node;
					AddToOpen(temp);
					closedList.emplace_back(temp);
					addedNodes = true;
				}
				firstCheckTrue = true;
			}
			if (tempCurr->GetRightNode() && !tempCurr->GetRightNode()->isTraversible)
			{

				if (tempCurr->GetBRNode() && tempCurr->GetBRNode()->isTraversible && !IsOnClosedList(&JPSNode(tempCurr->GetBRNode(), DIR_NONE)))
				{

					JPSNode* temp = new JPSNode(tempCurr, DIR_BOTTOMRIGHT);
					temp->node->parent = myStartNode->node;
					AddToOpen(temp);
					closedList.emplace_back(temp);
					addedNodes = true;
				}
				if (firstCheckTrue)
				{
					tempCurr->parent = nullptr;
					break;
				}
			}


			if (tempCurr == _endNode || (tempCurr->GetTLNode() == _endNode) || (tempCurr->GetBRNode() == _endNode))
			{
				JPSNode* goalTemp = new JPSNode(_endNode, DIR_NONE);
				goalTemp->node->parent = myStartNode->node;
				openList.emplace_back(goalTemp);
				break;
			}

			tempCurr = tempCurr->GetBLNode();
			//if (!addedNodes)
			//{
			//	tempCurr = tempCurr->GetBLNode();
			//}
			//else
			//{
			//	JPSNode* temp = new JPSNode(tempCurr, DIR_BOTTOMLEFT);
			//	temp->node->parent = myStartNode->node;
			//	AddToOpen(temp);
			//	closedList.emplace_back(temp);
			//	break;
			//}

		}
 	};
	auto iterateBR = [&openList, &closedList, &renderCopy, &iterateDown, &iterateRight, &AddToOpen, &IsOnClosedList](JPSNode* myStartNode, GOGridNode* _endNode)
	{

		GOGridNode* tempCurr = myStartNode->node;
		if (tempCurr->GetBRNode() && tempCurr->GetBRNode()->isTraversible && !IsOnClosedList(&JPSNode(tempCurr->GetBRNode(), DIR_NONE)))
		{
			tempCurr = tempCurr->GetBRNode();
		}
		while (tempCurr && tempCurr->isTraversible && !IsOnClosedList(&JPSNode(tempCurr, DIR_NONE)))
		{

			bool addedNodes = false;

			tempCurr->parent = myStartNode->node;
			JPSNode* tempNode = new JPSNode(tempCurr, DIR_NONE);
			if (iterateDown(tempNode, _endNode))
			{
				JPSNode* temp = new JPSNode(tempCurr, DIR_TOPLEFT);
				temp->node->parent = myStartNode->node;
				AddToOpen(temp);
				closedList.emplace_back(temp);
				addedNodes = true;
			}
			delete tempNode;
			tempNode = new JPSNode(tempCurr, DIR_NONE);
			if (iterateRight(tempNode, _endNode))
			{
				JPSNode* temp = new JPSNode(tempCurr, DIR_TOPLEFT);
				temp->node->parent = myStartNode->node;
				AddToOpen(temp);
				closedList.emplace_back(temp);
				addedNodes = true;
			}
			delete tempNode;
			SDL_SetRenderDrawColor(renderCopy, 255, 255, 0, 255);
			SDL_RenderFillRect(renderCopy, &tempCurr->destRect);
			SDL_SetRenderDrawColor(renderCopy, 0, 0, 0, 255);
			SDL_RenderPresent(renderCopy);

			//check below and to our side, as this is a diagonal check
			bool firstCheckTrue = false;
			if (tempCurr->GetTopNode() && !tempCurr->GetTopNode()->isTraversible)
			{

				if (tempCurr->GetTRNode() && tempCurr->GetTRNode()->isTraversible && !IsOnClosedList(&JPSNode(tempCurr->GetTRNode(), DIR_NONE)))
				{

					JPSNode* temp = new JPSNode(tempCurr, DIR_TOPRIGHT);
					temp->node->parent = myStartNode->node;
					AddToOpen(temp);
					closedList.emplace_back(temp);
					addedNodes = true;
				}
				firstCheckTrue = true;
			}
			if (tempCurr->GetLeftNode() && !tempCurr->GetLeftNode()->isTraversible)
			{

				if (tempCurr->GetBLNode() && tempCurr->GetBLNode()->isTraversible && !IsOnClosedList(&JPSNode(tempCurr->GetBLNode(), DIR_NONE)))
				{

					JPSNode* temp = new JPSNode(tempCurr, DIR_BOTTOMLEFT);
					temp->node->parent = myStartNode->node;
					AddToOpen(temp);
					closedList.emplace_back(temp);
					addedNodes = true;
				}
				if (firstCheckTrue)
				{
					tempCurr->parent = nullptr;
					break;
				}
			}


			if (tempCurr == _endNode || (tempCurr->GetTRNode() == _endNode) || (tempCurr->GetBLNode() == _endNode))
			{
				JPSNode* goalTemp = new JPSNode(_endNode, DIR_NONE);
				goalTemp->node->parent = myStartNode->node;
				openList.emplace_back(goalTemp);
				break;
			}

			tempCurr = tempCurr->GetBRNode();
			//if (!addedNodes)
			//{
			//	tempCurr = tempCurr->GetBRNode();
			//}
			//else
			//{
			//	JPSNode* temp = new JPSNode(tempCurr, DIR_BOTTOMRIGHT);
			//	temp->node->parent = myStartNode->node;
			//	AddToOpen(temp);
			//	closedList.emplace_back(temp);
			//	break;
			//}

		}
	};

	auto executeJPSNode = [&](JPSNode* _inputNode)
	{
		switch (_inputNode->dir)
		{
		case DIR_TOPLEFT:
			iterateTL(_inputNode, endNode);
			break;
		case DIR_TOPRIGHT:
			iterateTR(_inputNode, endNode);
			break;
		case DIR_BOTTOMLEFT:
			iterateBL(_inputNode, endNode);
			break;
		case DIR_BOTTOMRIGHT:
			iterateBR(_inputNode, endNode);
			break;
		case DIR_UP:
			iterateUp(_inputNode, endNode);
			break;
		case DIR_DOWN:
			iterateDown(_inputNode, endNode);
			break;
		case DIR_LEFT:
			iterateLeft(_inputNode, endNode);
			break;
		case DIR_RIGHT:
			iterateRight(_inputNode, endNode);
			break;
		default:
			break;
		}
	};
	JPSNode* currNode = nullptr;

	while (openList.size() > 0)
	{	
		currNode = openList[0];
		openSize = openList.size();
		float gCost, hCost;
		//gCost = ManhattanDist(openList[0]->node, startNode);
		gCost = GetGCost(currNode, startNode);
		hCost = ManhattanDist(openList[0]->node, endNode);
		float fCost = gCost + hCost;
		openList[0]->node->gCost = gCost;
		openList[0]->node->hCost = hCost;
		float lowestGCost = gCost;
		float lowestfCost = fCost;
		int lowestIndex = 0;
		for (size_t i = 1; i < openSize; i++)
		{
			//gCost = ManhattanDist(openList[i]->node, startNode);
			gCost = GetGCost(currNode, startNode);
			hCost = ManhattanDist(openList[i]->node, endNode);

			float fCost = gCost + hCost;

			openList[i]->node->gCost = gCost;
			openList[i]->node->hCost = hCost;

			if (fCost <= openList[lowestIndex]->node->GetFCost() && hCost < openList[lowestIndex]->node->hCost)
			{
				currNode = openList[i];
				lowestGCost = gCost;
				lowestfCost = fCost;
				lowestIndex = i;
			}
		}

		if (currNode->node == endNode)
		{
			break;
		}
		executeJPSNode(openList[lowestIndex]);
		closedList.emplace_back(currNode);
		openList.erase(openList.begin() + lowestIndex);

		for (size_t i = 0; i < openList.size(); i++)
		{
			SDL_SetRenderDrawColor(renderer, 100, 255, 100, 255);
			SDL_RenderFillRect(renderer, &openList[i]->node->destRect);
		}


		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderPresent(renderer);
	}


	if (currNode->node == endNode)
	{
		while (currNode->node != startNode && currNode->node != nullptr)
		{
			Vec2 currPos = currNode->node->GetNodePos();
			returnList.emplace_back(currPos);
			currNode->node = currNode->node->parent;
		}
	}

	returnList.emplace_back(startNode->GetNodePos());
	std::reverse(returnList.begin(), returnList.end());

	return returnList;
}
////////////////////////////////////////////////////////////////////////////////////


GOGridNode * PathfindManager::GetNearestNode(Vec2 _position)
{
	if (searchGridCopy.size() > 0)
	{
		int xAlong = _position.x / cellWidth;
		int yAlong = _position.y / cellHeight;

		int yCells = screenWidth / cellWidth;//how many cells are there per row.

		int cellIndex = (yCells * yAlong) + xAlong;

		if (cellIndex > searchGridCopy.size() - 1) cellIndex = searchGridCopy.size() - 1;
		if (cellIndex < 0) cellIndex = 0;

		return searchGridCopy[cellIndex];
	}
	else return nullptr;
}

int PathfindManager::ManhattanDist(GOGridNode * _start, GOGridNode * _target)
{
	bool sameX = false;
	bool sameY = false;

	if (_start->GetNodePos().x == _target->GetNodePos().x) sameX = true;
	if (_start->GetNodePos().y == _target->GetNodePos().y) sameY = true;

	int score = 0;

	GOGridNode* currNode = _start;

	while (sameX == false)
	{
		Vec2 thisPos = currNode->GetNodePos();
		Vec2 endPos = _target->GetNodePos();
		//are we going to be moving left, or right?
		if (thisPos.x > endPos.x)//need to move left
		{
			//gotta find the connected node on our left
			std::vector<GOGridNode*> connectedNodes = currNode->GridGetConnectedNodes();
			int numNodes = connectedNodes.size();
			for (size_t i = 0; i < numNodes; i++)
			{
				Vec2 connectedNodePos = connectedNodes[i]->GetNodePos();
				if (connectedNodePos.x < thisPos.x && connectedNodePos.y == thisPos.y)
				{
					currNode = connectedNodes[i];
					score += 10;
					break;
				}
			}
		}
		else if (thisPos.x < endPos.x)//need to move right
		{
			//gotta find the connected node on our right
			std::vector<GOGridNode*> connectedNodes = currNode->GridGetConnectedNodes();
			int numNodes = connectedNodes.size();
			for (size_t i = 0; i < numNodes; i++)
			{
				Vec2 connectedNodePos = connectedNodes[i]->GetNodePos();
				if (connectedNodePos.x > thisPos.x && connectedNodePos.y == thisPos.y)
				{
					currNode = connectedNodes[i];
					score += 10;
					break;
				}
			}
		}
		else//if numbers are equal
		{
			sameX = true;
		}
	}


	while (sameY == false)
	{
		Vec2 thisPos = currNode->GetNodePos();
		Vec2 endPos = _target->GetNodePos();
		//are we going to be moving up, or down?
		if (thisPos.y < endPos.y)//need to move down
		{
			//gotta find the connected node below us
			std::vector<GOGridNode*> connectedNodes = currNode->GridGetConnectedNodes();
			int numNodes = connectedNodes.size();
			for (size_t i = 0; i < numNodes; i++)
			{
				Vec2 connectedNodePos = connectedNodes[i]->GetNodePos();
				if (connectedNodePos.x == thisPos.x && connectedNodePos.y > thisPos.y)
				{
					currNode = connectedNodes[i];
					score += 10;
					break;
				}
			}
		}
		else if (thisPos.y > endPos.y)//need to move right
		{
			//gotta find the connected node above us
			std::vector<GOGridNode*> connectedNodes = currNode->GridGetConnectedNodes();
			int numNodes = connectedNodes.size();
			for (size_t i = 0; i < numNodes; i++)
			{
				Vec2 connectedNodePos = connectedNodes[i]->GetNodePos();
				if (connectedNodePos.x == thisPos.x && connectedNodePos.y < thisPos.y)
				{
					currNode = connectedNodes[i];
					score += 10;
					break;
				}
			}
		}
		else//if numbers are equal
		{
			sameY = true;
		}
	}


	return score;
}

float PathfindManager::EuclideanDist(GOGridNode * _start, GOGridNode * _target)
{
	Vec2 startPos = _start->GetNodePos();
	Vec2 targetPos = _target->GetNodePos();

	float dist = startPos.Distance(targetPos);
	float relativeVal = dist / _start->destRect.w;
	relativeVal *= 9.5f;

	return relativeVal;
}

int PathfindManager::GetGCost(GOGridNode * node)
{
	//when going through, the g cost will be the g cost of the node's parent + the additional distance
	int gCost = 0;
	
	if (node->parent && node->parent->gCost != 99999)
	{
		gCost += node->parent->gCost;
		std::vector<GOGridNode*> connectedNodes = node->parent->GridGetConnectedNodes();
		int numNodes = connectedNodes.size();
		for (size_t i = 0; i < numNodes; i++)
		{
			//find our node
			if (connectedNodes[i] == node)
			{
				Vec2 parentPos = node->parent->GetNodePos();
				Vec2 nodePos = node->GetNodePos();
				if (nodePos.x != parentPos.x)
				{
					if (nodePos.y != parentPos.y)//if true, we have moved diagonally
					{
						gCost += 13;
					}
					else//if false, we have moved laterally
					{
						gCost += 10;
					}
				}
				else//if 
				{
					gCost += 10;
				}
			}
		}
	}

	node->gCost = gCost;

	return gCost;
}

int PathfindManager::GetGCost(JPSNode * node, GOGridNode* startNode)
{
	//when going through, the g cost will be the g cost of the node's parent + the additional distance
	int gCost = 0;

	if (node->node)
	{
		gCost += node->node->parent->gCost;
	}
	
	gCost += ManhattanDist(startNode, node->node);

	return gCost;
}

void PathfindManager::DrawPathfindLists(std::vector<GOGridNode*> *_openList, std::vector<GOGridNode*> *_closedList, Vec2 *startPos, Vec2 *goalPos)
{
	//SDL_RenderClear(renderer);

	typedef std::chrono::milliseconds MS;
	int openSize = _openList->size();
	std::vector<GOGridNode*> openList = *_openList;
	for (size_t i = 0; i < openSize; i++)
	{
		Vec2 pos = Vec2(openList[i]->destRect.x, openList[i]->destRect.y);
		//float dist = pos.Distance(*goalPos);
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		//SDL_SetRenderDrawColor(renderer, 0, 255, 0, ((i+1) * 10));
		//SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		SDL_RenderFillRect(renderer, &openList[i]->destRect);

	}
	MS time = MS(2);
	std::this_thread::sleep_for(time);

	int closedSize = _closedList->size();
	std::vector<GOGridNode*> closedList = *_closedList;
	for (size_t i = 0; i < closedSize; i++)
	{
		Vec2 pos = Vec2(closedList[i]->destRect.x, closedList[i]->destRect.y);
		//float dist = pos.Distance(*goalPos);
		SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		//SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
		SDL_RenderFillRect(renderer, &closedList[i]->destRect);
	}
	std::this_thread::sleep_for(time);

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderPresent(renderer);
}
