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
		//string model = "(GS (t 0.00) (pm BeforeKickOff))";
		std::smatch match;
		//regex pattern("\((time|GS|B|P)(\\s\(\\w+ [0-9.]+\))");
		std::regex pattern("\\((time|GS|B|P)\\s(\\(.*\\))\\)");
		std::regex_search(message, match, pattern);
		std::string sHeader = match[1];
		std::string sBody = match[2];
		std::regex pattern2("(\\w+[a-zA-Z.0-9\\s]+)");
		if (sHeader == "GS")
		{
			double system_time;
			size_t index = sBody.find(" ");
			system_time = std::stod(sBody.substr(index + 1));
		}
		else if(sHeader=="time")
		{
			//double gametime;
			//std::string gamemode;
			
			for (std::sregex_iterator it(sBody.begin(), sBody.end(), pattern2), end_it; it != end_it; ++it)
			{
				//std::cout << it->str() << std::endl;
				std::string info = it->str();
				size_t index = (int)info.find(" ");
				std::string time_str = info.substr(index + 1);
				gametime = std::stod(time_str);
			}
		}
		else if (sHeader == "B")
		{
			/*std::smatch match2;
			if (std::regex_search(sBody, match2, pattern2))
			{
				std::string 
				
			}*/
			std::vector<double> ball_pos;
			size_t index = sBody.find(" ");
			while (index != sBody.length()-1)
			{

				size_t end = sBody.find(" ", index+1);
				if (end == std::string::npos)
				{
					end = sBody.length()-1;
				}
				std::string position = sBody.substr(index + 1, end);
				ball_pos.push_back(std::stod(position));
				index = end;
			}
		}
		else if (sHeader == "P")
		{
			std::vector<std::string> inform;
			//std::regex pattern2("(\\w+\\s[a-zA-Z.0-9\\s]+)");
			for (std::sregex_iterator it(sBody.begin(), sBody.end(), pattern2), end_it; it != end_it; ++it)
			{
				//std::cout << it->str() << std::endl;
				std::string info = it->str();
				inform.push_back(info);
			}
			std::string sTeam = inform[0];
			int playerID = std::stoi(inform[1].substr(2));
			std::vector<double> positon;
			getPosition(inform[2], position);
		}
		
}

void SoccerNao::getPosition(std::string str, std::vector<double>& pos)
{
	size_t index = str.find(" ");
	while (index != str.length() - 1)
	{
		size_t end = str.find(" ", index + 1);
		if (end == std::string::npos)
		{
			end = str.length() - 1;
		}
		std::string position = str.substr(index + 1, end);
		pos.push_back(std::stod(position));
		index = end;
	}
}