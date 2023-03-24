#include "soccernao.h"
#include <Windows.h>
#include <algorithm>
#include <utility>
#include "../global/global.h"

#define CONFIG_PATH		"../config.ini"

#define ROLE_STRIKER		0
#define ROLE_DEFENDER		1
#define ROLE_GOAL_KEEPER	2

#define ACTION_NONE			0
#define ACTION_KICK			1
#define ACTION_PASS			2
#define ACTION_DRIBBLE		3
#define ACTION_RUN			4
#define ACTION_INTERCEPT	5

#define TEAM_LEFT			0
#define TEAM_RIGHT			1

#define STRIKER_NUM			1

#define SQ(x) (x)*(x)
#define DIST2D(vec1 , vec2) (SQ(vec1[0] - vec2[0]) + SQ(vec1[1] - vec2[1]))
#define DOT(vec1,vec2)	((vec1[0] - vec2[0]) * (vec1[1] - vec2[1]))
#define MAX(x,y)	(x > y ? x : y)
#define MIN(x,y)	(x < y ? x : y)

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
	
	//read configuration
	char value[25] = { 0 };
	GetIniKeyString("SoccerField", "PitchLength", CONFIG_PATH, value);
	pitchVec[0] = atof(value) * 0.5;

	GetIniKeyString("SoccerField", "PitchWidth", CONFIG_PATH, value);
	pitchVec[1] = atof(value) * 0.5;

	GetIniKeyString("SoccerField", "PenaltyAreaLength", CONFIG_PATH, value);
	penaltyVec[0] = pitchVec[0] - atof(value);

	GetIniKeyString("SoccerField", "PenaltyAreaWidth", CONFIG_PATH, value);
	penaltyVec[1] = atof(value) * 0.5;

	GetIniKeyString("SoccerField", "GoalAreaLength", CONFIG_PATH, value);
	goalAreaVec[0] = pitchVec[0] - atof(value);

	GetIniKeyString("SoccerField", "GoalAreaWidth", CONFIG_PATH, value);
	goalAreaVec[1] = atof(value) * 0.5;

	GetIniKeyString("SoccerField", "GoalDepth", CONFIG_PATH, value);
	goalVec[0] = pitchVec[0] + atof(value);

	GetIniKeyString("SoccerField", "GoalWidth", CONFIG_PATH, value);
	goalVec[1] = atof(value) * 0.5;

	GetIniKeyString("SoccerField", "GoalHeight", CONFIG_PATH, value);
	goalZ = atof(value);

	GetIniKeyString("SoccerField", "PenaltyMarkDistance", CONFIG_PATH, value);
	penaltyMarkX = atof(value);

	GetIniKeyString("SoccerField", "BallDiameter", CONFIG_PATH, value);
	ballDiameter = atof(value);

	GetIniKeyString("SoccerField", "CentreCircleDiameter", CONFIG_PATH, value);
	centreD = atof(value);

	GetIniKeyString("Game", "PlayerNum", CONFIG_PATH, value);
	teamPlayerNum = atoi(value);

	//init player position vector
	playerPositions.resize(teamPlayerNum * 2);
	for (int i = 0; i < playerPositions.size(); i++)
	{
		playerPositions[i].resize(3);
	}

	vPlayerInfo.resize(teamPlayerNum * 2);
	vDist2Rnk.resize(teamPlayerNum * 2);
	vDist2RnkTeam.resize(teamPlayerNum);

	//get robot information
	playerId = std::stod(getName());
	playerTeam = playerId >= teamPlayerNum ? TEAM_RIGHT : TEAM_LEFT;

	pemitter = getEmitter("emitter");
	preceiver = getReceiver("receiver");
	preceiver->enable(TIME_STEP);
	preceiver->setChannel(-1);
}

void SoccerNao::soccerPaser()
{
	
	//update player info
	std::vector<std::pair<int, double>> _dist2Pair;
	for (int i = 0; i < playerPositions.size(); i++)
	{
		double _dist2ToBall = DIST2D(playerPositions[i + playerTeam * teamPlayerNum], ballPosition);
		vPlayerInfo[i].dist2ToBall = _dist2ToBall;
		_dist2Pair.push_back(std::make_pair(i, _dist2ToBall));
	}

	std::sort(_dist2Pair.begin(), _dist2Pair.end(), [=](std::pair<int, double> elem1, std::pair<int, double> elem2) {return elem1.second > elem2.second; });

	vDist2RnkTeam.clear();
	for (int i = 0; i < vDist2Rnk.size(); i++)
	{
		int id = _dist2Pair[i].first;
		vDist2Rnk[i] = id;
		if (isSameTeam(id))
			vDist2RnkTeam.push_back(id);
	}

	//update player action
	for (int i = 0; i < teamPlayerNum; i++)
	{
		int _playerId = playerTeam * teamPlayerNum + i;
		//striker
		if (isStriker(_playerId))
		{
			if (gameMode == GM_PLAY_ON)
			{
				//The striker will be goal focussed when the team has the ball
				//The striker got the ball
				if (isClosest2Ball(_playerId))
				{
					if (canShoot(_playerId))
					{
						//kick toward the goal
						setPlayerAction(_playerId, ACTION_KICK);

						double vec[2] = { 0.0, 0.0 };
						getBestShootVec(_playerId, vec);
						setPlayerActionParam(_playerId, vec);
					}
					else
					{
						//dribble the ball to another place
						getBestShootPlace(_playerId, vPlayerInfo[_playerId].actionParam);
					}
				}
				//Team got the ball
				else if (isTeamGotBall())
				{
					//Cooperate with teammates to pass the ball
					setPlayerAction(_playerId, ACTION_RUN);
					//The action param will be set by teammats;
				}
				//When the team does not have the ball then the striker behaviour will seek to intercept and gain control of the ball
				else if (isClosest2BallTeam(_playerId))
				{
					//run towards the ball
					setPlayerAction(_playerId, ACTION_RUN);
				}
				else
				{
					//wait for a pass from teammate
					setPlayerAction(_playerId, ACTION_RUN);
				}
			}
			else if (isKickOffTeam(_playerId))
			{
				double vec[2] = { 0.0,0.0 };
				getBestKickOffVec(_playerId, vec);
				setPlayerAction(_playerId, ACTION_KICK);
				setPlayerActionParam(_playerId, vec);
			}
			else if (isKickOffDefenderTeam(_playerId))
			{

			}
			else if (isThrowInTeam(_playerId))
			{


			}
			else if (isThrowInDefenderTeam(_playerId))
			{


			}
			else if (isCornerKickTeam(_playerId))
			{


			}
			else if (isCornerKickDefenderTeam(_playerId))
			{


			}
		}
		else if (isDefender(_playerId))
		{
			if (gameMode == GM_PLAY_ON)
			{
				//The range of the defender could extend depending on the current situation of play
				if (isClosest2Ball(_playerId))
				{
					//pass the ball to the closest striker
					int strikerId = getClosestStrikerId(_playerId);
					if (canPassBall(_playerId, strikerId))
					{
						vPlayerInfo[_playerId].action = ACTION_PASS;
						setPlayerActionParam(_playerId, &playerPositions[strikerId][0]);
					}
					else
					{
						double fVec[2] = { 0,0 };
						double tVec[2] = { 0,0 };

						getBestPassBallPlace(_playerId, &playerPositions[_playerId][0], strikerId, &playerPositions[strikerId][0]);
						setPlayerAction(_playerId, ACTION_RUN);
						setPlayerActionParam(_playerId, fVec);
						setPlayerActionParam(strikerId, tVec);
					}
				}
				else if (isTeamGotBall())
				{
					double vec[2] = { 0,0 };
					setPlayerAction(_playerId, ACTION_RUN);
					getDefenderAttackPosition(_playerId, vec);
					setPlayerActionParam(_playerId, vec);
				}
				else
				{
					//run towards the ball
					setPlayerAction(_playerId, ACTION_RUN);
					setPlayerActionParam(_playerId, ballPosition);
				}
			}
			else if (isKickOffTeam(_playerId))
			{

			}
			else if (isKickOffDefenderTeam(_playerId))
			{

			}
			else if (isThrowInTeam(_playerId))
			{


			}
			else if (isThrowInDefenderTeam(_playerId))
			{


			}
			else if (isCornerKickTeam(_playerId))
			{


			}
			else if (isCornerKickDefenderTeam(_playerId))
			{


			}
		}
		else if (isGoalKeeper(_playerId))
		{
			if (gameMode == GM_PLAY_ON)
			{
				if (isClosest2Ball(_playerId))
				{
					//pass to closest player
					int closestDefenderId = getClosestDefenderId(_playerId);
					if (canPassBall(_playerId, closestDefenderId))
					{
						setPlayerAction(_playerId, ACTION_PASS);
						setPlayerActionParam(_playerId, &playerPositions[closestDefenderId][0]);
					}
					else
					{
						double fVec[2] = { 0,0 };
						double tVec[2] = { 0,0 };

						getBestPassBallPlace(_playerId, &playerPositions[_playerId][0], closestDefenderId, &playerPositions[closestDefenderId][0]);
						setPlayerAction(_playerId, ACTION_RUN);
						setPlayerActionParam(_playerId, fVec);
						setPlayerActionParam(closestDefenderId, tVec);
					}
				}
				else if (isBallInPenaltyArea(_playerId))
				{
					//run to the ball
					setPlayerAction(_playerId, ACTION_RUN);
					setPlayerActionParam(_playerId, ballPosition);
				}
			}
			else if (isKickOffTeam(_playerId))
			{


			}
			else if (isKickOffDefenderTeam(_playerId))
			{

			}
			else if (isThrowInTeam(_playerId))
			{


			}
			else if (isThrowInDefenderTeam(_playerId))
			{


			}
			else if (isCornerKickTeam(_playerId))
			{


			}
			else if (isCornerKickDefenderTeam(_playerId))
			{


			}
		}
	}
	
}

void SoccerNao::run()
{
	int i = 0;

	while (step(16) != -1)
	{
		//read message buffer
		if (receive_message())
		{
			read_message();
		}

		soccerPaser();

		std::cout << ">> game mode: " << mPlayMode[gameMode] << std::endl;
		std::cout << ">> player action: " << vPlayerInfo[playerId].action << std::endl;
		std::cout << ">> player action param: " << vPlayerInfo[playerId].actionParam[0] << " " << vPlayerInfo[playerId].actionParam[1] << std::endl;


		//check if need stand 
		if (need_stand())
			do_the_correct(0);
		else
		{
			if (vPlayerInfo[playerId].action == ACTION_RUN)
			{
				move(vPlayerInfo[playerId].actionParam);
			}
			else if (vPlayerInfo[playerId].action == ACTION_PASS)
			{
				kick_towards(vPlayerInfo[playerId].actionParam, ballPosition);
			}
			else if (vPlayerInfo[playerId].action == ACTION_KICK)
			{
				kick_towards(vPlayerInfo[playerId].actionParam, ballPosition);
			}
		}
	}
}

bool SoccerNao::receive_message()
{
	if (preceiver->getQueueLength() == 0)
	{
		Sleep(50);
		return false;
	}
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
			//std::cout << "hello" << std::endl;
		}
		//messages.push(message);
		preceiver->nextPacket();
	}
	if(sign)
	{
		messages.push(message);
	}
	return true;
}

void SoccerNao::getPosition(std::string str, double* pos)
{
	int end = 0, index = 0, i = 0;
	while (index < str.length())
	{
		end = str.find(" ", index);
		if (end == std::string::npos)
		{
			end = str.length();
		}
		pos[i] = std::stod(str.substr(index, end));
		index = end + 1;
		i++;
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
		std::regex pattern2("(\\w+[a-zA-Z.0-9\\s\\-]+)");
		if (sHeader == "time")
		{
			double system_time;
			size_t index = sBody.find(" ");
			system_time = std::stod(sBody.substr(index + 1));
		}
		else if (sHeader == "GS")
		{
			for (std::sregex_iterator it(sBody.begin(), sBody.end(), pattern2), end_it; it != end_it; ++it)
			{
				std::string info = it->str();
				size_t index = info.find(" ");
				std::string title = info.substr(0, index);
				std::string tail = info.substr(index+1);
				if(title=="time")
				{
					gametime = std::stod(tail);
				}
				else if(title=="pm")
				{
					for (auto it = mPlayMode.begin(); it != mPlayMode.end(); ++it)
					{
						if (it->second == tail)
						{
							gameMode = it->first;
							break;
						}
					}
				}
			}
		}
		//visual sensor
		else if (sHeader == "See")
		{
			int sign = 0;
			for (std::sregex_iterator it(sBody.begin(), sBody.end(), pattern2), end_it; it != end_it; ++it)
			{
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
					size_t first_cut = info.find(" ");
					std::string title = info.substr(0, first_cut);
					std::string tail = info.substr(first_cut + 1);
					if (title == "ccs")
					{
						ball_position.clear();
						getPosition(tail, ballPosition);
					}
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
						//std::vector<double> o_position;
						if (seq >= 0)
						{
							getPosition(tail, &playerPositions[seq][0]);
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
	//return;
}

void SoccerNao::sendMessage(std::string header, std::string content)
{
	pemitter->setChannel(-1);
	std::string mes = "hello" + player_number;
	pemitter->send(&mes, mes.length());
}

bool SoccerNao::check_message(std::string message)
{
	std::smatch match;
	//regex pattern("\((time|GS|B|P)(\\s\(\\w+ [0-9.]+\))");
	std::regex pattern1("pm\\s(\\w+)");
	//std::cout << message << "check" << std::endl;
	if (std::regex_search(message, match, pattern1))
	{
		std::string s1 = match[1];
		if (s1 != mPlayMode[gameMode])
		{
			return true;
		}
	}
	return false;
}


inline bool SoccerNao::isStriker(int id)
{
	return (id - teamPlayerNum * playerTeam) < STRIKER_NUM;
}

inline bool SoccerNao::isDefender(int id)
{
	return (id - teamPlayerNum * playerTeam) > STRIKER_NUM && id % teamPlayerNum != 0;
}

inline bool SoccerNao::isGoalKeeper(int id)
{
	return id % teamPlayerNum == 0;
}

inline bool SoccerNao::isTeamLeft(int id)
{
	return id < teamPlayerNum;
}

inline bool SoccerNao::isTeamRight(int id)
{
	return id >= teamPlayerNum;
}

inline bool SoccerNao::isSameTeam(int id)
{
	return ((playerTeam == TEAM_LEFT && isTeamLeft(id)) || (playerTeam == TEAM_RIGHT && isTeamRight(id)));
}

inline bool SoccerNao::isKickOffTeam(int id)
{
	return ((gameMode == GM_KICK_OFF_LEFT && isTeamLeft(id)) || (gameMode == GM_KICK_OFF_RIGHT && isTeamRight(id)));
}

inline bool SoccerNao::isKickOffDefenderTeam(int id)
{
	return ((gameMode == GM_KICK_OFF_LEFT && !isTeamLeft(id)) || (gameMode == GM_KICK_OFF_RIGHT && !isTeamRight(id)));
}

inline bool SoccerNao::isThrowInTeam(int id)
{
	return ((gameMode == GM_THROW_IN_LEFT && isTeamLeft(id)) || (gameMode == GM_THROW_IN_RIGHT && isTeamRight(id)));
}

inline bool SoccerNao::isThrowInDefenderTeam(int id)
{
	return ((gameMode == GM_THROW_IN_LEFT && !isTeamLeft(id)) || (gameMode == GM_THROW_IN_RIGHT && !isTeamRight(id)));
}

inline bool SoccerNao::isCornerKickTeam(int id)
{
	return ((gameMode == GM_CORNER_KICK_LEFT && isTeamLeft(id)) || (gameMode == GM_THROW_IN_RIGHT && isTeamRight(id)));
}

inline bool SoccerNao::isCornerKickDefenderTeam(int id)
{
	return ((gameMode == GM_CORNER_KICK_LEFT && !isTeamLeft(id)) || (gameMode == GM_THROW_IN_RIGHT && !isTeamRight(id)));
}

inline bool SoccerNao::isClosest2Ball(int id)
{
	return (vDist2Rnk[0] == id);
}

inline bool SoccerNao::isClosest2BallTeam(int id)
{
	return (vDist2RnkTeam[0] == id);
}

bool SoccerNao::isClosest2BallDefender(int id)
{
	int _id = 0;
	for (int i = 0; i < vDist2RnkTeam.size(); i++)
	{
		if (isDefender(vDist2RnkTeam[_id]))
		{
			_id = vDist2RnkTeam[_id];
			break;
		}
	}
	return _id == id;
}

inline bool SoccerNao::isPlayerReach(int id, Vec2D vec)
{
	return DIST2D(vec, playerPositions[id]) <= ballDiameter;
}

inline bool SoccerNao::isTeamGotBall()
{
	return ((isTeamLeft(vDist2Rnk[0]) && playerTeam == TEAM_LEFT) ||
		((isTeamRight(vDist2Rnk[0]) && playerTeam == TEAM_RIGHT)));
}

inline bool SoccerNao::isBallInPenaltyArea(int id)
{
	if (isTeamLeft(id))
		return (ballPosition[0] < -penaltyVec[0] && ballPosition[1] < penaltyVec[1] && ballPosition[1] > -penaltyVec[1]);
	return (ballPosition[0] > penaltyVec[0] && ballPosition[1] < penaltyVec[1] && ballPosition[1] > -penaltyVec[1]);
}

inline bool SoccerNao::canShoot(int id)
{
	double goalVec[2] = { 0.0,0.0 };
	goalVec[1] = isTeamLeft(id) ? pitchVec[0] : -pitchVec[0];
	return DIST2D(playerPositions[id], goalVec) <= 5;
}

inline bool SoccerNao::canPassBall(int fId, int tId)
{
	return DIST2D(playerPositions[fId], playerPositions[tId]) <= 5;
}

inline void SoccerNao::getBestShootPlace(int id, Vec2D vec)
{
	vec[0] = isTeamLeft(id) ? goalAreaVec[0] : -goalAreaVec[0];
	vec[1] = 0.0;
}

inline void SoccerNao::getBestShootVec(int id, Vec2D vec)
{
	vec[0] = isTeamLeft(id) ? pitchVec[0] : -pitchVec[0];
	vec[1] = 0.0;
}

inline void SoccerNao::getBestPassBallPlace(int fId, Vec2D fVec, int tId, Vec2D tVec)
{
	memcpy(fVec, &playerPositions[tId][0], sizeof(&playerPositions[tId][0]));
	memcpy(tVec, &playerPositions[fId][0], sizeof(&playerPositions[fId][0]));
}

inline void SoccerNao::getBestKickOffVec(int id, Vec2D vec)
{
	vec[1] = ballPosition[1];
	vec[0] = isTeamLeft(id) ? (ballPosition[0] + 1) : (ballPosition[0] - 1);
}

inline void SoccerNao::getBestThrowInVec(int id, Vec2D vec)
{
	vec[0] = ballPosition[0];
	vec[1] = ballPosition[1] > 0 ? (pitchVec[1] - 1) : (pitchVec[1] + 1);
}

inline void SoccerNao::getBestCornerKickVec(int id, Vec2D vec)
{
	vec[0] = ballPosition[0] > 0 ? pitchVec[0] - 1 : -pitchVec[0] + 1;
	vec[1] = ballPosition[1] > 0 ? pitchVec[1] - 1 : -pitchVec[1] + 1;
}

void SoccerNao::getDefenderAttackPosition(int id, Vec2D pos)
{
	if (id == 1)
	{
		pos[0] = ballPosition[0] - 3;
		pos[1] = ballPosition[1] + 3;

	}
	else if (id == 2)
	{
		pos[0] = ballPosition[0] - 3;
		pos[1] = ballPosition[1] - 3;
	}
	else if (id == teamPlayerNum + 1)
	{
		pos[0] = ballPosition[0] + 3;
		pos[1] = ballPosition[1] + 3;
	}
	else if (id == teamPlayerNum + 2)
	{
		pos[0] = ballPosition[0] + 3;
		pos[1] = ballPosition[1] - 3;
	}
}

int SoccerNao::getClosest2BallDefender()
{
	int id = 0;
	for (int i = 0; i < vDist2RnkTeam.size(); i++)
	{
		if (isDefender(vDist2RnkTeam[i]))
		{
			id = isDefender(vDist2RnkTeam[i]);
			break;
		}
	}
	return id;
}

int SoccerNao::getClosest2BallStriker()
{
	int id = 0;
	for (int i = 0; i < vDist2RnkTeam.size(); i++)
	{
		if (isStriker(vDist2RnkTeam[i]))
		{
			id = isStriker(vDist2RnkTeam[i]);
			break;
		}
	}
	return id;
}

int SoccerNao::getClosestStrikerId(int id)
{
	std::map<int, double> mStrikerList;

	for (int i = 0; i < vDist2RnkTeam.size(); i++)
	{
		if (isStriker(vDist2RnkTeam[i]))
			mStrikerList[vDist2RnkTeam[i]] = DIST2D(playerPositions[id], playerPositions[vDist2RnkTeam[i]]);
	}

	int strikerId = 0;
	double dist = 1000.0;
	std::map<int, double>::iterator iter;
	
	for (iter = mStrikerList.begin(); iter != mStrikerList.end(); iter++)
	{
		if (iter->second < dist)
		{
			dist = iter->second;
			strikerId = iter->first;
		}
	}
	return strikerId;
}

int SoccerNao::getClosestDefenderId(int id)
{
	std::map<int, double> mDefenderList;

	for (int i = 0; i < vDist2RnkTeam.size(); i++)
	{
		if (isDefender(vDist2RnkTeam[i]))
			mDefenderList[vDist2RnkTeam[i]] = DIST2D(playerPositions[id], playerPositions[vDist2RnkTeam[i]]);
	}

	int defenderId = 0;
	double dist = 1000.0;
	std::map<int, double>::iterator iter;

	for (iter = mDefenderList.begin(); iter != mDefenderList.end(); iter++)
	{
		if (iter->second < dist)
		{
			dist = iter->second;
			defenderId = iter->first;
		}
	}
	return defenderId;
}

inline void SoccerNao::setPlayerAction(int id, int action)
{
	vPlayerInfo[id].action = action;
}

inline void SoccerNao::setPlayerActionParam(int id, Vec2D param)
{
	memcpy(vPlayerInfo[id].actionParam, param, sizeof(vPlayerInfo[id].actionParam));
}
