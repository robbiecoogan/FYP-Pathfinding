#pragma once

#include "SDL_image.h"
#include "SDL.h"
#include <string>
#include <iostream>
#include <vector>

struct Vec2
{
	float x, y;

	Vec2()
	{
		x = 0;
		y = 0;
	}

	Vec2(float inputX, float inputY)
	{
		x = inputX;
		y = inputY;
	}

	Vec2(const Vec2 &copy)
	{
		x = copy.x;
		y = copy.y;
	}

	///<summary>Converts the Vec2 into a normalized vector. This is permanent. If a temporary version is required, use 'Normalized()' instead.</summary>
	void Normalize()
	{
		float vecLen = std::sqrt((x * x) + (y * y));
		x = x / vecLen;
		y = y / vecLen;
	}

	float Magnitude()
	{
		return (std::sqrt((x * x) + (y * y)));
	}

	///<summary>Returns a copy of the Vec2 as a normalized vector. This is not permanent. If a permanent version is required, use 'Normalize()' instead.</summary>
	Vec2 Normalized()
	{
		float vecLen = std::sqrt((x * x) + (y * y));
		if (vecLen != 0)
		{
			return Vec2(x / vecLen, y / vecLen);
		}
		else
		{
			return Vec2(0, 0);
		}
	}

	///<summary>Returns the absolute distance between this Vec2 and the Vec2 passed as an argument</summary>
	float Distance(Vec2 other)
	{
		float xDiff = x - other.x;
		xDiff *= xDiff;
		float yDiff = y - other.y;
		yDiff *= yDiff;

		return std::abs( std::sqrt(xDiff + yDiff) );
	}

	float Dot(Vec2 &other)
	{
		return ((x * other.x) + (y * other.y));
	}

	float AngleBetween(Vec2 &other)
	{
		float dotVal = this->Dot(other);
		dotVal += (dotVal == 0) ? 1 : 0;
		float valll = ( (dotVal) / ( (this->Magnitude())*(other.Magnitude())));
		return valll * 57.2958;
	}

	static Vec2 Zero()
	{
		return Vec2(0,0);
	}

	Vec2 operator * (const float &input)
	{
		return Vec2(x * input, y * input);
	}

	void operator *= (const float &input)
	{
		x *= input;
		y *= input;
	}

	void operator += (const float &input)
	{
		x += input;
		y += input;
	}

	void operator += (const Vec2 &input)
	{
		x += input.x;
		y += input.y;
	}

	Vec2 operator + (const Vec2 &input)
	{
		return Vec2(x + input.x, y + input.y);
	}

	Vec2 operator - (const Vec2 &input)
	{
		return Vec2(x - input.x, y - input.y);
	}

	bool operator == (const Vec2 &input)
	{
		return (input.x == x && input.y == y);
	}

	bool operator != (const Vec2 &input)
	{
		return (!(input.x == x && input.y == y));
	}

};

enum steerType
{
	STEERTYPE_NONE,
	STEERTYPE_ARRIVE,
	STEERTYPE_SEEK,
};

enum GOType
{
	NA,//decided to use NA rather than NONE, as it would be too similar to 'NODE'
	BOID,
	NODE,
	GRIDNODE,
};

class GameObject
{

public:
	GameObject(Vec2 position, SDL_Window* gameWindow, SDL_Renderer* gameRenderer);
	~GameObject();

	virtual void Update();
	virtual void Draw();

	///<summary>uses the file directory argument to find a file and sets this as it's texture. Returns false if failed.</summary>
	bool SetSprite(std::string fileDir);

	SDL_Rect destRect;

	GOType myType;

	Vec2 position;

protected:


	SDL_Texture * texture;

	SDL_Window* window;
	SDL_Renderer* renderer;

	std::string spriteDir;

	SDL_Rect sourceRect;


	Vec2 velocity;
	Vec2 acceleration;
};

class GONode;
class GOGridNode;

class GOBoid : public GameObject
{
public:
	GOBoid(Vec2 position, SDL_Window* gameWindow, SDL_Renderer* gameRenderer);
	~GOBoid();

	//AI steering behaviour
	void Arrive(Vec2 arriveLocation);
	void Seek(Vec2 seekLocation);
	void PathfindASTAR(Vec2 position, std::vector<GOGridNode*> _searchGrid);
	void PathfindJPS(Vec2 position, std::vector<GOGridNode*> _searchGrid);
	void SetSearchSpace(std::vector<GOGridNode*> _searchSpace) { searchSpace = _searchSpace; }

	void AddNodeToPath(GONode* _node) { currPath.emplace_back(_node); }
	void SetEndGoal(Vec2 _endGoal);

	void Update() override;
	void Draw() override;

private:

	void HandleCollisions();

	steerType mySteerType;
	Vec2 arriveLoc;
	std::vector<GONode*> currPath;
	std::vector<Vec2> pathfindList;
	std::vector<GOGridNode*> searchSpace;
	int currPathIndex;

	void* _pathfindManager;

	float maxSpeed;

	float neighborhoodArea;
	float disengageDist;

	Vec2 goalPos;

	float DegreeToRadian(float degrees)
	{
		return (degrees * 0.0174533);//simple conversion to radians
	}

	template <typename T>
	void AbsClamp(T absMin, T absMax, T& val)
	{
		float tempVal = val;
		bool isNegative = false;
		if (tempVal < 0)
		{
			tempVal = -tempVal;
			isNegative = true;
		}
		if (tempVal > absMax) val = absMax;
		else if (tempVal < absMin) val = absMin;

		if (val > 0 && isNegative) val = -val;
	}
};

class GONode : public GameObject
{
public:
	GONode(Vec2 position, SDL_Window* gameWindow, SDL_Renderer* gameRenderer);
	~GONode();

	virtual void AddConnection(GONode* node);

	virtual std::vector<GONode*> GetConnectedNodes() { return connectedNodes; }

	bool IsCurrentlyClicked() { return mouseDown; }

	void Update() override;
	void Draw() override;

protected:

	bool mouseDown;

	//std::bitset

private:
	std::vector<GONode*> connectedNodes;

};

class GOGridNode : public GONode
{
public:
	GOGridNode(SDL_Window* gameWindow, SDL_Renderer* gameRenderer, Vec2 position, int cellWidth, int cellHeight);
	~GOGridNode();

	void AddConnection(GOGridNode* node);
	std::vector<GOGridNode*> GridGetConnectedNodes() { return connectedNodes; }

	Vec2 GetNodePos() { return Vec2(destRect.x + (destRect.w/2), destRect.y + (destRect.h/2)); }//returns centre points of node

	GOGridNode* GetTopNode();///<summary>Returns a pointer to the node above this one, if such a node exists. If not, a nullptr is returned</summary>
	GOGridNode* GetLeftNode();///<summary>Returns a pointer to the node to the left of this one, if such a node exists. If not, a nullptr is returned</summary>
	GOGridNode* GetRightNode();///<summary>Returns a pointer to the node to the right this one, if such a node exists. If not, a nullptr is returned</summary>
	GOGridNode* GetBottomNode();///<summary>Returns a pointer to the node below this one, if such a node exists. If not, a nullptr is returned</summary>
	GOGridNode* GetTLNode();///<summary>Returns a pointer to the node to the top-left this one, if such a node exists. If not, a nullptr is returned</summary>
	GOGridNode* GetTRNode();///<summary>Returns a pointer to the node to the top-right this one, if such a node exists. If not, a nullptr is returned</summary>
	GOGridNode* GetBLNode();///<summary>Returns a pointer to the node to the bottom-left this one, if such a node exists. If not, a nullptr is returned</summary>
	GOGridNode* GetBRNode();///<summary>Returns a pointer to the node to the bottom-right this one, if such a node exists. If not, a nullptr is returned</summary>

	void Update() override;
	void Draw() override;

	bool IsCurrentlyClicked() { return mouseDown; }

	//////////

	bool isTraversible;

	GOGridNode* parent;//used for backtracing when pathfinding;
	float gCost;
	float hCost;
	float GetFCost() { return (gCost + hCost); }

private:
	std::vector<GOGridNode*> connectedNodes;

};