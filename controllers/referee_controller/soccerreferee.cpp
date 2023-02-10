#include "soccerreferee.h"

using namespace webots;

const std::vector<std::string> vPlayer1Def = {
	"RED_0", "RED_1", "RED_2", "RED_GOAL"
};

const std::vector<std::string> vPlayer2Def = {
	"BLUE_0", "BLUE_1", "BLUE_2", "BLUE_GOAL"
};

const std::string ballDef = "Ball";

SoccerReferee::SoccerReferee()
{
	for (int i = 0; i < vPlayer1Def.size(); i++)
	{
		Node* pNode = getFromDef(vPlayer1Def[i]);
		if (pNode == NULL)
		{
			std::cerr << "No DEF " << vPlayer1Def[i] << "node found in the current world file" << std::endl;
			exit(1);
		}
		vPlayerNodes1.push_back(pNode);
	}

	for (int i = 0; i < vPlayer2Def.size(); i++)
	{
		Node* pNode = getFromDef(vPlayer2Def[i]);
		if (pNode == NULL)
		{
			std::cerr << "No DEF " << vPlayer2Def[i] << "node found in the current world file" << std::endl;
			exit(1);
		}
		vPlayerNodes2.push_back(pNode);
	}

	pBallNode = getFromDef(ballDef);
	if (pBallNode == NULL)
	{
		std::cerr << "No DEF " << ballDef << "node found in the current world file" << std::endl;
		exit(1);
	}

}

void SoccerReferee::run()
{
	




}

