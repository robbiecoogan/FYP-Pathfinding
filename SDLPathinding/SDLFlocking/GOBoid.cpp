#include "GameObject.h"
#include "EventManager.h"
#include <math.h>
#include <algorithm>
#include "PathfindManager.h"

GOBoid::GOBoid(Vec2 position, SDL_Window * gameWindow, SDL_Renderer * gameRenderer) : GameObject(position, gameWindow, gameRenderer)
{
	int screenWidth, screenHeight;
	SDL_GetWindowSize(window, &screenWidth, &screenHeight);

	myType = GOType::BOID;
	velocity = Vec2(-1.0f, 1.0f);
	currPathIndex = -1;
	maxSpeed = 150.0f;
	disengageDist = 20.0f;
	goalPos = Vec2(-1, -1);
	_pathfindManager = new PathfindManager(window, renderer, screenWidth, screenHeight, 40, 40);
}

GOBoid::~GOBoid()
{

}

void GOBoid::SetEndGoal(Vec2 _endGoal)
{
	goalPos = _endGoal;
}

void GOBoid::Update()
{
	acceleration = Vec2::Zero();

	int mouseX, mouseY;
	SDL_GetMouseState(&mouseX, &mouseY);

	if (mySteerType != STEERTYPE_NONE && currPathIndex >= 0 && currPathIndex <= pathfindList.size()-1 && pathfindList.size() > 0)//when we are arriving at a point, we will constantly call the arrive function. The function will change our state to normal once it has arrived
	{
		if (currPathIndex == pathfindList.size() - 1)
		{
			Vec2 seekPos = pathfindList[currPathIndex];
			Arrive(seekPos);
		}
		else
		{
			Vec2 seekPos = pathfindList[currPathIndex];
			Seek(seekPos);
		}
	}

	std::vector<SDL_Event*> e = EventManager::getInstance().GetEvent(SDL_KEYDOWN);
	int eSize = e.size();
	if (eSize > 0)
	{
		for (size_t i = 0; i < eSize; i++)
		{
			if (e[i]->key.keysym.sym == SDLK_f)
			{
				PathfindASTAR(Vec2(mouseX, mouseY), searchSpace);
				if (pathfindList.size() > 0)
				{
					currPathIndex = 0;
					mySteerType = steerType::STEERTYPE_SEEK;
				}
			}
			if (e[i]->key.keysym.sym == SDLK_g)
			{
				PathfindJPS(Vec2(mouseX, mouseY), searchSpace);
				if (pathfindList.size() > 0)
				{
					currPathIndex = 0;
					mySteerType = steerType::STEERTYPE_SEEK;
				}
			}
		}
	}

	HandleCollisions();

	velocity.x += acceleration.x;
	velocity.y += acceleration.y;

	if (velocity.Magnitude() > maxSpeed)
	{
		Vec2 Dir = velocity.Normalized();
		velocity = (Dir * maxSpeed);
	}

	position += velocity * (EventManager::dTime/1000);

	velocity *= 0.98;

	destRect.x = (int)position.x;
	destRect.y = (int)position.y;
}


void GOBoid::Arrive(Vec2 inputArriveLocation)
{
	if (inputArriveLocation.x != -1)
	{
		arriveLoc = inputArriveLocation;
	}


	if (mySteerType != STEERTYPE_ARRIVE)
	{
		mySteerType = STEERTYPE_ARRIVE;
		arriveLoc = inputArriveLocation;
	}
	else
	{
		Vec2 centrePos = Vec2(destRect.x + (destRect.w / 2), destRect.y + (destRect.h / 2));
		Vec2 fwdVec = velocity.Normalized();

		Vec2 Dir(arriveLoc.x - centrePos.x, arriveLoc.y - centrePos.y);
		Dir.Normalize();

		float moveSpeed = Vec2(centrePos.x, centrePos.y).Distance(arriveLoc);
		moveSpeed *= EventManager::dTime;

		if (moveSpeed > maxSpeed) moveSpeed = maxSpeed;
		else if (moveSpeed < 0) moveSpeed == 0;

		velocity = Dir * (moveSpeed);

		if (centrePos.Distance(arriveLoc + fwdVec) < disengageDist)
		{
			//mySteerType = STEERTYPE_NONE;
			currPathIndex++;
		}
	}

}

void GOBoid::Seek(Vec2 inputSeekLocation)
{
	if (inputSeekLocation.x != -1)
	{
		arriveLoc = inputSeekLocation;
	}


	if (mySteerType != STEERTYPE_SEEK)
	{
		mySteerType = STEERTYPE_SEEK;
		arriveLoc = inputSeekLocation;
	}
	else
	{
		Vec2 centrePos = Vec2(destRect.x + (destRect.w / 2), destRect.y + (destRect.h / 2));
		Vec2 fwdVec = velocity.Normalized();

		Vec2 Dir(arriveLoc.x - centrePos.x, arriveLoc.y - centrePos.y);
		if (Dir != centrePos)
		{
			Dir.Normalize();

			float moveSpeed = 1000.0f;
			moveSpeed *= EventManager::dTime / 1000;
			acceleration = Dir * moveSpeed;

			if (centrePos.Distance(arriveLoc + fwdVec) < disengageDist)
			{
				//mySteerType = STEERTYPE_NONE;
				currPathIndex++;
			}
		}
	}
}

void GOBoid::PathfindASTAR(Vec2 position, std::vector<GOGridNode*> _searchGrid)
{
	Vec2 centrePos = Vec2(destRect.x + (destRect.w / 2), destRect.y + (destRect.h / 2));

	pathfindList = ((PathfindManager*)_pathfindManager)->AStarPathfind(centrePos, position, _searchGrid);
	//pathfindList = ((PathfindManager*)_pathfindManager)->JPSPathfind(centrePos, position, _searchGrid);
}

void GOBoid::PathfindJPS(Vec2 position, std::vector<GOGridNode*> _searchGrid)
{
	Vec2 centrePos = Vec2(destRect.x + (destRect.w / 2), destRect.y + (destRect.h / 2));

	//pathfindList = ((PathfindManager*)_pathfindManager)->AStarPathfind(centrePos, position, _searchGrid);
	pathfindList = ((PathfindManager*)_pathfindManager)->JPSPathfind(centrePos, position, _searchGrid);
}


void GOBoid::Draw()
{
	SDL_Point centre = { (destRect.w / 2), (destRect.h / 2) };

	Vec2 velAdj = velocity;
	velAdj.x += centre.x;
	velAdj.y += centre.y;

	int pfSize = pathfindList.size();
	if (pfSize > 0)
	{
		for (size_t i = currPathIndex-1; i < pfSize - 1; i++)
		{
			Vec2 startPos, endPos;
			startPos = pathfindList[i], endPos = pathfindList[i + 1];
			SDL_RenderDrawLine(renderer, startPos.x, startPos.y, endPos.x, endPos.y);
		}
	}

	float angleToFace = atan2(velAdj.y - centre.y, velAdj.x - centre.x);
	angleToFace = angleToFace * (180 / (3.14159));//convert to degrees

	//SDL_RenderCopyEx(renderer, texture, &sourceRect, &destRect, right.AngleBetween(velocity), &centre, SDL_FLIP_NONE);
	SDL_RenderCopyEx(renderer, texture, &sourceRect, &destRect, angleToFace, &centre, SDL_FLIP_NONE);
}

void GOBoid::HandleCollisions()
{
	int screenWidth, screenHeight;
	SDL_GetWindowSize(window, &screenWidth, &screenHeight);

	Vec2 centre = Vec2(destRect.x + (destRect.w / 2), destRect.y + (destRect.h / 2));

	GOGridNode* currNode = ((PathfindManager*)_pathfindManager)->GetNearestNode(centre);

	if (currNode)
	{
		std::vector<GOGridNode*> connectedNodes = currNode->GridGetConnectedNodes();
		int numNodes = connectedNodes.size();

		for (size_t i = 0; i < numNodes; i++)
		{
			Vec2 connectedNodePos = connectedNodes[i]->GetNodePos();
			if (connectedNodePos.Distance(centre) < 30 && !connectedNodes[i]->isTraversible)
			{
				Vec2 dir = centre - connectedNodePos;
				float force = 100.0f / connectedNodePos.Distance(centre);//inverse
				acceleration += dir * force;
			}
		}
	}
}
