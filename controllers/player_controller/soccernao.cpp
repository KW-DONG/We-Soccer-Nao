#include "soccernao.h"
#include <Windows.h>
#include "../global/global.h"

#define CONFIG_PATH		"../config.ini"

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
	pemitter = getEmitter("emitter");
	preceiver = getReceiver("receiver");
	
	preceiver->enable(TIME_STEP);
	gamemode = mPlayMode[GM_NONE];
	player_number = pemitter->getChannel();
	//player_number = 0;

	team = player_number % 4;
	//other_player = new std::vector<double>[7];
	role = player_number == 3 ? 0 : 1;

}

void SoccerNao::run()
{
	//std::thread t1(&SoccerNao::receive_message, this);
	int i = 0;
	while (step(TIME_STEP) != -1)
	{
		i++;
		
		receive_message();

		read_message();

		if (gamemode == mPlayMode[GM_NONE])
		{
			continue;
		}
		else if (gamemode == mPlayMode[GM_BEFORE_KICK_OFF])
		{

		}
		else if (gamemode == mPlayMode[GM_PLAY_ON])
		{
			// finding the ball and 
			double ball[] = { ball_position[0], ball_position[1], ball_position[2] };
			move(ball);
		}
		std::cout << i << std::endl;
	}
	/*if (t1.joinable())
	{
		t1.join();
	}*/
}

void SoccerNao::init(int number)
{
	//player_number = number;
}

void SoccerNao::receive_message()
{
	std::cout << preceiver->getQueueLength() << std::endl;
	while(preceiver->getQueueLength() > 0)
	{
		std::cout << "hello" << std::endl;
		std::string message = (char*)preceiver->getData();
		//string model = "(GS (t 0.00) (pm BeforeKickOff))";
		messages.push(message);
		std::cout << "hello" << message << std::endl;
		preceiver->nextPacket();
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

void SoccerNao::read_message()
{
	if (!messages.empty())
	{

		std::string message = messages.front();
		std::smatch match;
		//regex pattern("\((time|GS|B|P)(\\s\(\\w+ [0-9.]+\))");
		std::regex pattern("\\((time|GS|See|B|P|KB|HB)\\s(\\(.*\\))\\)");
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
		else if (sHeader == "time")
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
		else if (sHeader == "See")
		{
			int sign = 0;
			for (std::sregex_iterator it(sBody.begin(), sBody.end(), pattern2), end_it; it != end_it; ++it)
			{
				//cout << it->str() << endl;
				std::string info = it->str();
				if (info == "B ")
				{
					sign = 0;
				}
				else if (info == "P ")
				{
					sign = 1;
				}
				else if (sign == 0)
				{
					//std::vector<double> ball_position;
					for (std::sregex_iterator it_b(info.begin(), info.end(), pattern2), end_itb; it_b != end_itb; ++it_b)
					{
						std::string str = info.substr(info.find(" ") + 1);
						int end = 0, index = 0;
						while (index < str.length())
						{
							end = str.find(" ", index);
							if (end == std::string::npos)
							{
								end = str.length();
							}
							ball_position.push_back(std::stod(str.substr(index, end)));
							index = end + 1;
						}
					}
				}
				else if (sign == 1)
				{
					std::vector<double>* player_position = new std::vector<double>;
					for (std::sregex_iterator it_p(info.begin(), info.end(), pattern2), end_itp; it_p != end_itp; ++it_p)
					{
						std::string str = info.substr(info.find(" ") + 1);
						int end = 0, index = 0;
						int i = 0;
						while (index < str.length())
						{
							end = str.find(" ", index);
							if (end == std::string::npos)
							{
								end = str.length();
							}
							player_position->push_back(std::stod(str.substr(index, end)) );
							index = end + 1;
						}
					}
					other_player.push_back(*player_position);
				}
			}
		}
		else if (sHeader == "KB")
		{
			int player_id;
			size_t index = sBody.find(" ");
			player_id = std::stoi(sBody.substr(index + 1));
		}
	}
	messages.pop();
}

void SoccerNao::send_message(std::string header, std::string content)
{
	
}