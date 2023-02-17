#include "soccernao.h"
#include <Windows.h>

using namespace webots;

std::map<int, std::string> mPlayMode = {
	{GM_BEFORE_KICK_OFF, "BeforeKickOff"},
	{GM_PLAY_ON, "PlayOn"},
	{GM_GAME_OVER, "GameOver"},
	{GM_NONE, "None"},
	{GM_KICK_OFF_LEFT, "KickOffLeft"},
	{GM_KICK_OFF_RIGHT, "KickOffRight"},
	{GM_THROW_IN_LEFT, "ThrowInLeft"},
	{GM_THROW_IN_RIGHT, "ThrowInRight"},
	{GM_GOAL_KICK_LEFT, "GoalKickLeft"},
	{GM_GOAL_KICK_RIGHT, "GoalKickRight"},
	{GM_FREE_KICK_LEFT, "FreeKickLeft"},
	{GM_FREE_KICK_RIGHT, "FreeKickRight"},
	{GM_PENALTY_KICK_LEFT, "PenaltyKickLeft"},
	{GM_PENALTY_KICK_RIGHT, "PenaltyKickRight"},
	{GM_CORNER_KICK_LEFT, "CornerKickLeft"},
	{GM_CORNER_KICK_RIGHT, "CornerKickRight"},
};

SoccerNao::SoccerNao()
{

}

void SoccerNao::run()
{
	
	while (step(TIME_STEP) != -1)
	{
		
	}
}

void SoccerNao::init(int number)
{
	player_number = number;
}

void SoccerNao::receive_message()
{
	if (pReceiver->getQueueLength() > 0)
	{
		std::string message = (char*)pReceiver->getData();

		if (message == mPlayMode[GM_BEFORE_KICK_OFF])
		{
			init()
		}
	}
}

int SoccerNao::readHeader(std::string& src, std::string& dst, int offset)
{
		
}