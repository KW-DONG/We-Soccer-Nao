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
	preceiver->setChannel(-1);
	std::cout << preceiver->getChannel() << std::endl;
	gamemode = mPlayMode[GM_NONE];
	player_number = pemitter->getChannel();
	//player_number = 0;
	other_player = new double* [7];
	for (int i = 0; i < 7; i++)
	{
		other_player[i] = new double[3];
	}
	team = player_number % 4;
	//other_player = new std::vector<double>[7];
	role = player_number == 3 ? 0 : 1;

}

void SoccerNao::run()
{
	//std::thread t1(&SoccerNao::get_message, this);
	int i = 0;

	while (step(TIME_STEP) != -1)
	{
		i++;
		//send_message("1", "2");
		if (receive_message())
		{
			read_message();
		}
		std::cout << gamemode << std::endl;
		if (gamemode == mPlayMode[GM_NONE])
		{
			//continue;
		}
		else if (gamemode == mPlayMode[GM_BEFORE_KICK_OFF])
		{

		}
		else if (gamemode == mPlayMode[GM_PLAY_ON])
		{
			// finding the ball and 
			//std::cout << "goal!" << std::endl;
			if (!ball_position.empty())
			{
				double ball[] = { ball_position[0], ball_position[1], ball_position[2] };
				/*std::cout << ball[0] << ball[1] << ball[2] << std::endl;
				std::cout << "cxk" << std::endl;*/
				move(ball);
				//std::cout << "goal!!!!" << std::endl;
			}
		}
		//std::cout << i << std::endl;
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

bool SoccerNao::receive_message()
{
	if (preceiver->getQueueLength() == 0)
	{
		Sleep(50);
		return false;
	}
	std::cout << preceiver->getQueueLength() << std::endl;
	std::string message;
	bool sign = true;
	while(preceiver->getQueueLength() > 0)
	{
		//std::cout << "hello" << std::endl;
		sign = true;
		message = (char*)preceiver->getData();
		//string model = "(GS (t 0.00) (pm BeforeKickOff))";s
		if (check_message(message))
		{
			messages.push(message);
			sign = false;
		}
		//std::cout << message+" controller" << std::endl;
		preceiver->nextPacket();
	}
	if(sign)
	{
		messages.push(message);
	}
	return true;
}

void SoccerNao::getPosition(std::string str, std::vector<double>& pos)
{
	int end = 0, index = 0;
	while (index < str.length())
	{
		end = str.find(" ", index);
		if (end == std::string::npos)
		{
			end = str.length();
		}
		/*std::cout << std::stod(str.substr(index, end)) << std::endl;
		std::cout << "in" << std::endl;*/
		pos.push_back(std::stod(str.substr(index, end)));
		index = end + 1;
	}
}

void SoccerNao::read_message()
{
	if (!messages.empty())
	{

		std::string message = messages.front();
		//std::cout << message + " read" << std::endl;
		std::smatch match;
		//regex pattern("\((time|GS|B|P)(\\s\(\\w+ [0-9.]+\))");
		std::regex pattern("\\((time|GS|See|B|P|KB|HB)\\s(\\(.*\\))\\)");
		std::regex_search(message, match, pattern);
		std::string sHeader = match[1];
		std::string sBody = match[2];
		std::regex pattern2("(\\w+[a-zA-Z.0-9\\s\\-]+)");
		if (sHeader == "time")
		{
			double system_time;
			size_t index = sBody.find(" ");
			system_time = std::stod(sBody.substr(index + 1));
		}
		else if (sHeader == "GS")
		{
			//double gametime;
			//std::string gamemode;

			for (std::sregex_iterator it(sBody.begin(), sBody.end(), pattern2), end_it; it != end_it; ++it)
			{
				//std::cout << it->str() << std::endl;
				std::string info = it->str();
				size_t index = info.find(" ");
				std::string title = info.substr(0, index);
				std::string tail = info.substr(index+1);
				if(title=="time")
				{
					gametime = std::stod(tail);
					std::cout << gametime << std::endl;
					std::cout << "time" << std::endl;
				}
				else if(title=="pm")
				{
					gamemode = tail;
				}
				//gametime = std::stod(time_str);
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
					/*for (std::sregex_iterator it_b(info.begin(), info.end(), pattern2), end_itb; it_b != end_itb; ++it_b)
					{
						size_t first_cut = info.find(" ");
						std::string title = info.substr(0, first_cut);
						std::string tail = info.substr(first_cut + 1);
						if (title == "css")
						{
							int end = 0, index = 0;
							while (index < tail.length())
							{
								end = tail.find(" ", index);
								if (end == std::string::npos)
								{
									end = tail.length();
								}
								ball_position.push_back(std::stod(tail.substr(index, end)));
								index = end + 1;
							}
						}
					}*/
					size_t first_cut = info.find(" ");
					std::string title = info.substr(0, first_cut);
					std::string tail = info.substr(first_cut + 1);
					//std::cout << "yes"+title << std::endl;
					if (title == "ccs")
					{
						//std::cout << "yes" << std::endl;
						getPosition(tail, ball_position);
					}
					/*for (int i = 0; i < ball_position.size(); i++)
					{
						std::cout << ball_position[i] << std::endl;
					}
					std::cout << "get ball" << std::endl;*/
				}
				else if (sign == 1)
				{
					int seq = -1;
					size_t first_cut = info.find(" ");
					std::string title = info.substr(0, first_cut);
					std::string tail = info.substr(first_cut + 1);
					if (title == "id")
					{
						seq = std::stoi(tail);
					}
					else if (title == "ccs")
					{
						std::vector<double> o_position;
						if (seq >= 0)
						{
							getPosition(tail, o_position);
							other_player[seq][0] = o_position[0];
							other_player[seq][1] = o_position[1];
							other_player[seq][2] = o_position[2];
						}
					}
					
				}
			}
		}
		else if (sHeader == "KB")
		{
			int player_id;
			size_t index = sBody.find(" ");
			player_id = std::stoi(sBody.substr(index + 1));
		}
		messages.pop();
	}
	return;
}

void SoccerNao::send_message(std::string header, std::string content)
{
	pemitter->setChannel(-1);
	std::string mes = "hello" + player_number;
	pemitter->send(&mes, mes.length());
}

bool SoccerNao::check_message(std::string message)
{
	std::smatch match;
	//regex pattern("\((time|GS|B|P)(\\s\(\\w+ [0-9.]+\))");
	std::regex pattern1("gm");
	return std::regex_match(message, match, pattern1);
}