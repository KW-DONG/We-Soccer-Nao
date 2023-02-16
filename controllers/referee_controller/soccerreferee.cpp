#include "soccerreferee.h"
#include <cstdlib>
#include <thread>
#include <Windows.h>

using namespace webots;

const std::vector<std::string> vPlayerDefLeft = {
	"R0", "R1", "R2", "RG"
};

const std::vector<std::string> vPlayerDefRight = {
	"B0", "B1", "B2", "BG"
};

const std::string ballDef = "Ball";

void _Node::updatePosition()
{
	memcpy(&translation, pNode->getField("translation"), sizeof(translation));
	memcpy(&rotation, pNode->getField("rotation"), sizeof(rotation));
}

void SoccerReferee::readConfig()
{
	
}

SoccerReferee::SoccerReferee()
{
	for (int i = 0; i < vPlayerDefLeft.size(); i++)
	{
		_Node _node;
		_node.pNode = getFromDef(vPlayerDefLeft[i]);
		if (_node.pNode == NULL)
		{
			std::cerr << "No DEF " << vPlayerDefLeft[i] << "node found in the current world file" << std::endl;
			exit(1);
		}
		vPlayerLeftNodes.push_back(_node);
	}

	for (int i = 0; i < vPlayerDefRight.size(); i++)
	{
		_Node _node;
		_node.pNode = getFromDef(vPlayerDefRight[i]);
		if (_node.pNode == NULL)
		{
			std::cerr << "No DEF " << vPlayerDefRight[i] << "node found in the current world file" << std::endl;
			exit(1);
		}
		vPlayerRightNodes.push_back(_node);
	}

	ballNode.pNode = getFromDef(ballDef);
	if (ballNode.pNode == NULL)
	{
		std::cerr << "No DEF " << ballDef << "node found in the current world file" << std::endl;
		exit(1);
	}

	pEmitter = getEmitter("emitter");
	pReceiver = getReceiver("receiver");
	pReceiver->enable(TIME_STEP);
	
}

void SoccerReferee::run()
{
	std::thread tReceiver(&threadReceiver);
	while (step(TIME_STEP) != -1)
	{
		readPosition();	
		autoReferee();

	}
	if(tReceiver.joinable())
	{
		tReceiver.join();
	}
}

void SoccerReferee::readPosition()
{
	for (int i = 0; i < vPlayerLeftNodes.size(); i++)
	{
		for (int i = 0; i < vPlayerLeftNodes.size(); i++)
			vPlayerLeftNodes[i].updatePosition();

		for (int i = 0; i < vPlayerRightNodes.size(); i++)
			vPlayerRightNodes[i].updatePosition();

		ballNode.updatePosition();
	}
}

void SoccerReferee::threadReceiver()
{
	while (step(TIME_STEP) != -1)
	{
		if (pReceiver->getQueueLength() > 0)
		{
			char* data = (char*)pReceiver->getData();
			pReceiver->nextPacket();
		}
	}
}

void SoccerReferee::autoReferee()
{
	if (currentState == PLAY_ON)
	{
		
	}
	else if (currentState == KICK_OFF_LEFT || KICK_OFF_RIGHT)
	{

	}
	else if (currentState == THROW_IN_LEFT)
	{

	}
	else if (currentState == THROW_IN_RIGHT)
	{

	}
	else if (currentState == GOAL_KICK_LEFT)
	{

	}
	else if (currentState == GOAL_KICK_RIGHT)
	{

	}
	else if (currentState == FREE_KICK_LEFT)
	{

	}
	else if (currentState == FREE_KICK_RIGHT)
	{

	}
	else if (currentState == PENALTY_KICK_LEFT)
	{

	}
	else if (currentState == PENALTY_KICK_RIGHT)
	{

	}
	else if (currentState == CORNER_KICK_LEFT)
	{

	}
	else if (currentState == CORNER_KICK_RIGHT)
	{

	}

}






