#include "GameObject.h"
#include "EventManager.h"

GOGridNode::GOGridNode(SDL_Window* gameWindow, SDL_Renderer* gameRenderer, Vec2 position, int cellWidth, int cellHeight) : GONode(position, gameWindow, gameRenderer)
{
	destRect.x = position.x;
	destRect.y = position.y;
	destRect.w = cellWidth;
	destRect.h = cellHeight;
	myType = GOType::GRIDNODE;
	isTraversible = true;
	parent = nullptr;
	gCost = 99999;
	hCost = 99999;
}

void GOGridNode::AddConnection(GOGridNode * _node)
{
	int connectedCount = connectedNodes.size();
	//need to ensure the node is not already on the list.
	for (size_t i = 0; i < connectedCount; i++)
	{
		if (_node == connectedNodes[i])
		{
			std::cout << "Cannot add same node twice.\n";
			return;
		}
	}

	connectedNodes.emplace_back(_node);//add the other node to this node's list.
		
}

GOGridNode * GOGridNode::GetTopNode()
{
	int connectedSize = connectedNodes.size();
	if (connectedSize == 8)//if this node is not on a wall or a corner
	{
		return connectedNodes[5];
	}
	else if (connectedSize == 5)//if this node is on a wall
	{
		if (connectedNodes[0]->destRect.y != destRect.y)//if this node is on the left wall
		{
			return connectedNodes[2];
		}
		else if (connectedNodes[4]->destRect.y != destRect.y)//if this node is on the right wall
		{
			return connectedNodes[4];
		}
		else if (connectedNodes[3]->destRect.y > destRect.y)//if this node is on the top wall
		{
			return nullptr;//no top node to return
		}
		else
		{
			return connectedNodes[2];
		}
	}
	else if (connectedSize == 3)//if this node is in a corner
	{
		if (connectedNodes[2]->destRect.y < destRect.y)//bottom right corner
		{
			return connectedNodes[2];
		}
		else if (connectedNodes[0]->destRect.y < destRect.y)//bottom left corner
		{
			return connectedNodes[0];
		}
		else//if we are either top left or top right node, we don't have a top, so return nullptr.
		{
			return nullptr;
		}
	}

	return nullptr;
}

GOGridNode * GOGridNode::GetLeftNode()
{
	int connectedSize = connectedNodes.size();
	if (connectedSize == 8)//if this node is not on a wall or a corner
	{
		return connectedNodes[0];
	}
	else if (connectedSize == 5)//if this node is on a wall
	{
		if (connectedNodes[0]->destRect.y != destRect.y)//if this node is on the left wall
		{
			return nullptr;
		}
		else//otherwise, the left node is always 0.
		{
			return connectedNodes[0];
		}
	}
	else if (connectedSize == 3)//if this node is in a corner
	{
		if (connectedNodes[0]->destRect.x == destRect.x)
		{
			return nullptr;
		}
		else
		{
			return connectedNodes[0];
		}
	}

	return nullptr;
}

GOGridNode * GOGridNode::GetRightNode()
{
	int connectedSize = connectedNodes.size();
	if (connectedSize == 8)//if this node is not on a wall or a corner
	{
		return connectedNodes[7];
	}
	else if (connectedSize == 5)//if this node is on a wall
	{
		if (connectedNodes[0]->destRect.y != destRect.y)//if this node is on the left wall
		{
			return connectedNodes[4];
		}
		else if (connectedNodes[4]->destRect.y != destRect.y)//if this node is on the right wall
		{
			return nullptr;
		}
		else if (connectedNodes[3]->destRect.y > destRect.y)//if this node is on the top wall
		{
			return connectedNodes[4];
		}
		else//if on bottom wall
		{
			return connectedNodes[4];
		}
	}
	else if (connectedSize == 3)//if this node is in a corner
	{
		if (connectedNodes[2]->destRect.x == destRect.y)//if this is true, we are either the top right or bottom right corner, meaning that we should return nullptr.
		{
			return nullptr;
		}
		else
		{
			return connectedNodes[2];
		}
	}

	return nullptr;
}

GOGridNode * GOGridNode::GetBottomNode()
{
	int connectedSize = connectedNodes.size();
	if (connectedSize == 8)//if this node is not on a wall or a corner
	{
		return connectedNodes[2];
	}
	else if (connectedSize == 5)//if this node is on a wall
	{
		if (connectedNodes[0]->destRect.y != destRect.y)//if this node is on the left wall
		{
			return connectedNodes[0];
		}
		else if (connectedNodes[4]->destRect.y != destRect.y)//if this node is on the right wall
		{
			return connectedNodes[2];
		}
		else if (connectedNodes[3]->destRect.y > destRect.y)//if this node is on the top wall
		{
			return connectedNodes[2];
		}
		else
		{
			return nullptr;
		}
	}
	else if (connectedSize == 3)//if this node is in a corner
	{
		if (connectedNodes[0]->destRect.y > destRect.y)//top left corner
		{
			return connectedNodes[0];
		}
		else if (connectedNodes[2]->destRect.y > destRect.y)//top right corner
		{
			return connectedNodes[2];
		}
		else
		{
			return nullptr;
		}
	}

	return nullptr;
}

GOGridNode * GOGridNode::GetTLNode()
{
	int connectedSize = connectedNodes.size();
	if (connectedSize == 8)//if this node is not on a wall or a corner
	{
		return connectedNodes[4];
	}
	else if (connectedSize == 5)//if this node is on a wall
	{
		if (connectedNodes[0]->destRect.y != destRect.y)//if this node is on the left wall
		{
			return nullptr;
		}
		else if (connectedNodes[4]->destRect.y != destRect.y)//if this node is on the right wall
		{
			return connectedNodes[3];
		}
		else if (connectedNodes[3]->destRect.y > destRect.y)//if this node is on the top wall
		{
			return nullptr;
		}
		else//bottom wall
		{
			return connectedNodes[1];
		}
	}
	else if (connectedSize == 3)//if this node is in a corner
	{
		if (connectedNodes[2]->destRect.y < destRect.y)//bottom right corner
		{
			return connectedNodes[1];
		}
		else
		{
			return nullptr;
		}
	}

	return nullptr;
}

GOGridNode * GOGridNode::GetTRNode()
{
	int connectedSize = connectedNodes.size();
	if (connectedSize == 8)//if this node is not on a wall or a corner
	{
		return connectedNodes[6];
	}
	else if (connectedSize == 5)//if this node is on a wall
	{
		if (connectedNodes[0]->destRect.y != destRect.y)//if this node is on the left wall
		{
			return connectedNodes[3];
		}
		else if (connectedNodes[4]->destRect.y != destRect.y)//if this node is on the right wall
		{
			return nullptr;
		}
		else if (connectedNodes[3]->destRect.y > destRect.y)//if this node is on the top wall
		{
			return nullptr;
		}
		else//bottom wall
		{
			return connectedNodes[3];
		}
	}
	else if (connectedSize == 3)//if this node is in a corner
	{
		if (connectedNodes[0]->destRect.y < destRect.y)//bottom left corner
		{
			return connectedNodes[1];
		}
		else
		{
			return nullptr;
		}
	}

	return nullptr;
}

GOGridNode * GOGridNode::GetBLNode()
{
	int connectedSize = connectedNodes.size();
	if (connectedSize == 8)//if this node is not on a wall or a corner
	{
		return connectedNodes[1];
	}
	else if (connectedSize == 5)//if this node is on a wall
	{
		if (connectedNodes[0]->destRect.y != destRect.y)//if this node is on the left wall
		{
			return nullptr;
		}
		else if (connectedNodes[4]->destRect.y != destRect.y)//if this node is on the right wall
		{
			return connectedNodes[1];
		}
		else if (connectedNodes[3]->destRect.y > destRect.y)//if this node is on the top wall
		{
			return connectedNodes[1];
		}
		else//bottom wall
		{
			return nullptr;
		}
	}
	else if (connectedSize == 3)//if this node is in a corner
	{
		if (connectedNodes[2]->destRect.y > destRect.y)//top right corner
		{
			return connectedNodes[1];
		}
		else
		{
			return nullptr;
		}
	}

	return nullptr;
}

GOGridNode * GOGridNode::GetBRNode()
{
	int connectedSize = connectedNodes.size();
	if (connectedSize == 8)//if this node is not on a wall or a corner
	{
		return connectedNodes[3];
	}
	else if (connectedSize == 5)//if this node is on a wall
	{
		if (connectedNodes[0]->destRect.y != destRect.y)//if this node is on the left wall
		{
			return connectedNodes[1];
		}
		else if (connectedNodes[4]->destRect.y != destRect.y)//if this node is on the right wall
		{
			return nullptr;
		}
		else if (connectedNodes[3]->destRect.y > destRect.y)//if this node is on the top wall
		{
			return connectedNodes[3];
		}
		else//bottom wall
		{
			return nullptr;
		}
	}
	else if (connectedSize == 3)//if this node is in a corner
	{
		if (connectedNodes[0]->destRect.y > destRect.y)//top right corner
		{
			return connectedNodes[1];
		}
		else
		{
			return nullptr;
		}
	}

	return nullptr;
}

void GOGridNode::Update()
{
}

void GOGridNode::Draw()
{
	//if (!isTraversible)
	//{
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

		Vec2 centrePos = GetNodePos();

		SDL_RenderDrawPoint(renderer, centrePos.x, centrePos.y);
		SDL_RenderDrawPoint(renderer, centrePos.x - 1, centrePos.y);
		SDL_RenderDrawPoint(renderer, centrePos.x + 1, centrePos.y);
		SDL_RenderDrawPoint(renderer, centrePos.x, centrePos.y - 1);
		SDL_RenderDrawPoint(renderer, centrePos.x, centrePos.y + 1);

		if (!isTraversible)
		{
			SDL_SetRenderDrawColor(renderer, 255, 0, 0, 100);
			SDL_RenderFillRect(renderer, &destRect);
		}

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderDrawRect(renderer, &destRect);
	/*}*/
}
