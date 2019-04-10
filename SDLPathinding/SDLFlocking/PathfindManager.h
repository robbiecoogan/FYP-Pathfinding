#pragma once

#include "GameObject.h"

enum Direction
{
	DIR_TOPLEFT,
	DIR_UP,
	DIR_TOPRIGHT,
	DIR_LEFT,
	DIR_RIGHT,
	DIR_BOTTOMLEFT,
	DIR_DOWN,
	DIR_BOTTOMRIGHT,
	DIR_NONE,
};

struct JPSNode
{
	GOGridNode* node;
	Direction dir;

	JPSNode() { node = nullptr; dir = DIR_NONE; };

	JPSNode(GOGridNode* _node, Direction _dir)
	{
		node = _node;
		dir = _dir;
	}
};


class PathfindManager
{
public:
	PathfindManager(SDL_Window* _window, SDL_Renderer* _renderer, int _screenWidth, int _screenHeight, int _cellWidth, int _cellHeight);
	~PathfindManager();

	std::vector<Vec2> AStarPathfind(Vec2 startPos, Vec2 goalPos, std::vector<GOGridNode*> searchGrid);//A star algorithm
	std::vector<Vec2> JPSPathfind(Vec2 startPos, Vec2 goalPos, std::vector<GOGridNode*> searchGrid);//Jump point search algorithm

	GOGridNode* GetNearestNode(Vec2 _position);

private:

	int ManhattanDist(GOGridNode* _start, GOGridNode* _target);
	float EuclideanDist(GOGridNode* _start, GOGridNode* _target);
	int GetGCost(GOGridNode* node);
	int GetGCost(JPSNode* node, GOGridNode* startNode);

	std::vector<GOGridNode*> searchGridCopy;

	int screenWidth, screenHeight, cellWidth, cellHeight;
	SDL_Window* window;
	SDL_Renderer* renderer;

	void DrawPathfindLists(std::vector<GOGridNode*> *_openList, std::vector<GOGridNode*> *_closedList, Vec2 *startPos, Vec2 *goalPos);
};
