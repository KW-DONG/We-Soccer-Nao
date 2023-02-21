#include "soccerreferee.h"
#include <cstdlib>
#include <thread>
#include <ctime>
#include <cstdlib>
#include <map>
#include "global.h"

using namespace webots;

#define CONFIG_PATH		"../config.ini"

const std::string ballDef = "Ball";

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

std::map<int, std::string> mTeamName = {
	{_PlayerNode::TEAM_LEFT, "teamLeft"},
	{_PlayerNode::TEAM_RIGHT, "teamRight"}
};

void _Node::updatePosition()
{
	memcpy(&translation, pNode->getField("translation"), sizeof(translation));
	memcpy(&rotation, pNode->getField("rotation"), sizeof(rotation));
}

SoccerReferee::SoccerReferee()
{	
	//soccer pitch
	char value[25] = { 0 };
	GetIniKeyString("SoccerField", "PitchLength", CONFIG_PATH, value);
	pitchVec[0] = atof(value) * 0.5;

	GetIniKeyString("SoccerField", "PitchWidth", CONFIG_PATH, value);
	pitchVec[1] = atof(value) * 0.5;

	GetIniKeyString("SoccerField", "PenaltyAreaLength", CONFIG_PATH, value);
	penaltyVec[0] = pitchVec[0] * 0.5 - atof(value);

	GetIniKeyString("SoccerField", "PenaltyAreaWidth", CONFIG_PATH, value);
	penaltyVec[0] = pitchVec[1] + atof(value) * 0.5;

	GetIniKeyString("SoccerField", "GoalAreaLength", CONFIG_PATH, value);
	goalAreaVec[0] = pitchVec[0] * 0.5 - atof(value);

	GetIniKeyString("SoccerField", "GoalAreaWidth", CONFIG_PATH, value);
	goalAreaVec[1] = pitchVec[1] + atof(value) * 0.5;

	GetIniKeyString("SoccerField", "GoalDepth", CONFIG_PATH, value);
	goalVec[0] = pitchVec[0] * 0.5 + atof(value);

	GetIniKeyString("SoccerField", "GoalWidth", CONFIG_PATH, value);
	goalVec[1] = pitchVec[1] + atof(value) * 0.5;

	GetIniKeyString("SoccerField", "GoalHeight", CONFIG_PATH, value);
	goalZ = atof(value);

	GetIniKeyString("SoccerField", "PenaltyMarkDistance", CONFIG_PATH, value);
	penaltyMarkX = atof(value);

	GetIniKeyString("SoccerField", "BallDiameter", CONFIG_PATH, value);
	ballDiameter = atof(value);

	GetIniKeyString("SoccerField", "CentreCircleDiameter", CONFIG_PATH, value);
	centreD = atof(value);

	//game
	GetIniKeyString("Game", "PlayerNum", CONFIG_PATH, value);
	playerNum = atoi(value);

	GetIniKeyString("Game", "DurationSec", CONFIG_PATH, value);
	gameDuration = atoi(value);

	//team
	static _PlayerNode _node;
	_node.isReady = false;
	_node.role = _PlayerNode::PLAYER;
	for (int i = 0; i < playerNum; i++)
	{	
		std::string key = "DEF" + std::to_string(i);
		GetIniKeyString("Team1", key.c_str(), CONFIG_PATH, value);
		_node.pNode = getFromDef(value);
		if (_node.pNode == NULL)
		{
			std::cerr << "No DEF " << value << " node found in the current world file" << std::endl;
			exit(1);
		}
		_node.id = i;
		_node.team = _PlayerNode::TEAM_LEFT;
		vPlayerNodes.push_back(_node);
	}
	vPlayerNodes[vPlayerNodes.size()].role = _PlayerNode::GOAL_KEEPER;

	for (int i = 0; i < playerNum; i++)
	{
		std::string key = "DEF" + std::to_string(i);
		GetIniKeyString("Team2", key.c_str(), CONFIG_PATH, value);
		_node.pNode = getFromDef(value);
		if (_node.pNode == NULL)
		{
			std::cerr << "No DEF " << value << " node found in the current world file" << std::endl;
			exit(1);
		}
		_node.id = i;
		_node.team = _PlayerNode::TEAM_RIGHT;
		vPlayerNodes.push_back(_node);
	}
	
	//ball
	GetIniKeyString("Ball", "Diameter", CONFIG_PATH, value);
	ballDiameter = atof(value);

	GetIniKeyString("Ball", "DEF", CONFIG_PATH, value);
	ballNode.pNode = getFromDef(value);

	pEmitter = getEmitter("emitter");
	pReceiver = getReceiver("receiver");
	pReceiver->enable(TIME_STEP);
}

void SoccerReferee::run()
{
	while (step(TIME_STEP) != -1)
	{
		readReceiver();
		readPosition();	
		if (flag != FLG_HANDBALL_LEFT && flag != FLG_HANDBALL_RIGHT)
			localReferee();
		lastGameMode = gameMode;
		stateDriver();

		//boardcast playmode
		if (lastGameMode != gameMode)
		{
			std::string msg = "(GS (time " + gameTime + ") (pm " + mPlayMode[gameMode] + "))";
			pEmitter->setChannel(-1);
			pEmitter->send(msg.c_str(), msg.size());
		}

		//send position
		std::string msg = "(See " + seeBall(msg);
		for (int i = 0; i < playerNum; i++)
		{
			msg = seePlayer(msg, i);
		}
		msg += ")";
		pEmitter->setChannel(-1);
		pEmitter->send(msg.c_str(), msg.size());
	}
}

void SoccerReferee::findNodeSection(_Node& node)
{
	if (node.translation[0] >= penaltyVec[0] &&
		node.translation[1] <= penaltyVec[1] &&
		node.translation[1] >= -penaltyVec[1])
	{
		if (node.translation[0] >= goalVec[0] &&
			node.translation[1] <= goalVec[1] &&
			node.translation[1] >= -goalVec[1])
			node.section = PS_LEFT_GOAL_AREA;
		else node.section = PS_LEFT_PENALTY_AREA;
	}
	else if (node.translation[0] <= -penaltyVec[0] &&
		node.translation[1] <= penaltyVec[1] &&
		node.translation[1] >= -penaltyVec[1])
	{
		if (node.translation[0] <= -goalVec[0] &&
			node.translation[1] <= goalVec[1] &&
			node.translation[1] >= -goalVec[1])
			node.section = PS_RIGHT_GOAL_AREA;
		else node.section = PS_RIGHT_PENALTY_AREA;
	}
	else if (node.translation[0] < 0)
		node.section = PS_LEFT_NORMAL;
	else
		node.section = PS_RIGHT_NORMAL;
}

void SoccerReferee::findBallSection()
{
	if (ballNode.translation[0] < -pitchVec[0] && ballNode.translation[0] > -goalVec[0] &&
		ballNode.translation[1] < goalVec[1] && ballNode.translation[1] > -goalVec[1] &&
		ballNode.translation[2] < goalZ)
		ballNode.section = PS_LEFT_GOAL;
	else if (ballNode.translation[0] < pitchVec[0] && ballNode.translation[0] < goalVec[0] &&
		ballNode.translation[1] < goalVec[1] && ballNode.translation[1] > -goalVec[1] &&
		ballNode.translation[2] < goalZ)
		ballNode.section = PS_LEFT_GOAL;
	//ball hit ground
	else if (ballNode.translation[2] <= ballDiameter)
	{
		if (ballNode.translation[0] < -pitchVec[0])
			ballNode.section = PS_OUT_LEFT_ENDLINE;
		else if (ballNode.translation[0] > pitchVec[0])
			ballNode.section = PS_OUT_RIGHT_ENDLINE;
		else if (ballNode.translation[1] < -pitchVec[1] || ballNode.translation[1] > pitchVec[1])
			ballNode.section = PS_OUT_SIDELINE;
	}
	else
	{
		findNodeSection(ballNode);
	}
}

void SoccerReferee::readPosition()
{
	for (int i = 0; i < vPlayerNodes.size(); i++)
	{
		vPlayerNodes[i].updatePosition();
		findNodeSection(vPlayerNodes[i]);
	}
	ballNode.updatePosition();
}

void SoccerReferee::readReceiver()
{
	if (pReceiver->getQueueLength() > 0)
	{
		std::string data = (char*)pReceiver->getData();
		std::string sHeader, sTeam, sId;
		int offset = readHeader(data, sHeader, 0);

		offset = readProperty(data, sTeam, offset);
		readProperty(data, sId, offset);
		lastBallKeeperId = atoi(sId.c_str());

		if (sHeader == "RD")
		{
			if (sTeam == TEAM_LEFT_NAME)
				vPlayerNodes[lastBallKeeperId].isReady = true;
			else if (sTeam == TEAM_RIGHT_NAME)
				vPlayerNodes[lastBallKeeperId + TEAM_SIZE].isReady = true;
		}
		else
		{
			if (sTeam == TEAM_LEFT_NAME)
			{
				lastBallKeeperPosition[0] = vPlayerNodes[lastBallKeeperId].translation[0];
				lastBallKeeperPosition[1] = vPlayerNodes[lastBallKeeperId].translation[1];
				lastBallKeeperRole = vPlayerNodes[lastBallKeeperId].role;
				lastBallKeeperTeam = _PlayerNode::TEAM_LEFT;
			}		
			else if (sTeam == TEAM_RIGHT_NAME)
			{
				lastBallKeeperPosition[0] = vPlayerNodes[lastBallKeeperId + TEAM_SIZE].translation[0];
				lastBallKeeperPosition[1] = vPlayerNodes[lastBallKeeperId + TEAM_SIZE].translation[1];
				lastBallKeeperRole = vPlayerNodes[lastBallKeeperId + TEAM_SIZE].role;
				lastBallKeeperTeam = _PlayerNode::TEAM_RIGHT;
			}
		
			if (sHeader == "HB")
			{
				if (lastBallKeeperTeam == _PlayerNode::TEAM_LEFT)
				{
					if (lastBallKeeperRole == _PlayerNode::GOAL_KEEPER)
						flag = FLG_KICKBALL_LEFT;
					else
						flag = FLG_HANDBALL_LEFT;
				}
				else if (sTeam == TEAM_RIGHT_NAME)
				{
					if (lastBallKeeperRole == _PlayerNode::GOAL_KEEPER)
						flag = FLG_KICKBALL_RIGHT;
					else
						flag = FLG_HANDBALL_RIGHT;
				}
			}
			else if (sHeader == "KB")
			{
				if (sTeam == TEAM_LEFT_NAME)
					flag = FLG_KICKBALL_LEFT;
				else
					flag = FLG_KICKBALL_RIGHT;
			}
		}
		pReceiver->nextPacket();
	}
}

void SoccerReferee::localReferee()
{		
	if (gameMode == GM_BEFORE_KICK_OFF)
	{
		bool isAllReady = true;
		for (int i = 0; i < vPlayerNodes.size(); i++)
		{
			if (!vPlayerNodes[i].isReady)
			{
				isAllReady = false;
				break;
			}
		}
		if (isAllReady)
			flag = FLG_PLAYER_READY;
	}
	else if (gameMode == GM_PLAY_ON)
	{
		if (ballNode.section == PS_LEFT_GOAL)
			flag = FLG_GOAL_RIGHT;
		else if (ballNode.section == PS_RIGHT_GOAL)
			flag = FLG_GOAL_LEFT;
		else if (ballNode.section == PS_OUT_SIDELINE)
		{
			if (lastBallKeeperTeam == _PlayerNode::TEAM_LEFT)
				flag = FLG_OUT_OF_SIDELINE_LEFT;
			else
				flag = FLG_OUT_OF_SIDELINE_RIGHT;
		}
		else if (ballNode.section == PS_OUT_LEFT_ENDLINE)
			flag = FLG_OUT_OF_ENDLINE_LEFT;
		else if (ballNode.section == PS_OUT_RIGHT_ENDLINE)
			flag = FLG_OUT_OF_ENDLINE_RIGHT;
		//offside
		else if (ballNode.section > PS_LEFT_GOAL || ballNode.section <= PS_LEFT_NORMAL)
		{
			
		}
		else if (ballNode.section > PS_RIGHT_GOAL || ballNode.section <= PS_RIGHT_NORMAL)
		{

		}
	}
	else if (gameMode == GM_KICK_OFF_LEFT)
	{
		if (flag == FLG_KICKBALL_LEFT)
			flag = FLG_COMPLETE;
	}
	else if (gameMode == GM_KICK_OFF_RIGHT)
	{
		if (flag == FLG_KICKBALL_RIGHT)
			flag = FLG_COMPLETE;
	}
	else if (gameMode == GM_THROW_IN_LEFT)
	{
		if (flag == FLG_KICKBALL_LEFT)
			flag = FLG_COMPLETE;
	}
	else if (gameMode == GM_THROW_IN_RIGHT)
	{
		if (flag == FLG_KICKBALL_RIGHT)
			flag = FLG_COMPLETE;
	}
	else if (gameMode == GM_GOAL_KICK_LEFT)
	{
		if (flag == FLG_KICKBALL_LEFT)
			flag = FLG_COMPLETE;
	}
	else if (gameMode == GM_GOAL_KICK_RIGHT)
	{
		if (flag == FLG_KICKBALL_RIGHT)
			flag = FLG_COMPLETE;
	}
	else if (gameMode == GM_FREE_KICK_LEFT)
	{
		if (flag == FLG_KICKBALL_LEFT)
			flag = FLG_COMPLETE;
	}
	else if (gameMode == GM_FREE_KICK_RIGHT)
	{
		if (flag == FLG_KICKBALL_RIGHT)
			flag = FLG_COMPLETE;
	}
	else if (gameMode == GM_PENALTY_KICK_LEFT)
	{
		if (flag == FLG_KICKBALL_LEFT)
			flag = FLG_COMPLETE;
	}
	else if (gameMode == GM_PENALTY_KICK_RIGHT)
	{
		if (flag == FLG_KICKBALL_RIGHT)
			flag = FLG_COMPLETE;
	}
	else if (gameMode == GM_CORNER_KICK_LEFT)
	{
		if (flag == FLG_KICKBALL_LEFT)
			flag = FLG_COMPLETE;
	}
	else if (gameMode == GM_CORNER_KICK_RIGHT)
	{
		if (flag == FLG_KICKBALL_RIGHT)
			flag = FLG_COMPLETE;
	}
}

void SoccerReferee::stateDriver()
{
	switch (flag)
	{
	case FLG_START:
		onStart();
		break;
	case FLG_OUT_OF_ENDLINE_LEFT:
		onOutOfEndLineLeft();
		break;
	case FLG_OUT_OF_ENDLINE_RIGHT:
		onOutOfEndLineRight();
		break;
	case FLG_OUT_OF_SIDELINE_LEFT:
		onOutOfSideLineLeft();
		break;
	case FLG_OUT_OF_SIDELINE_RIGHT:
		onOutOfSideLineRight();
		break;
	case FLG_OFFSIDE_LEFT:
		onOffsideLeft();
		break;
	case FLG_OFFSIDE_RIGHT:
		onOffsideRight();
		break;
	case FLG_GOAL_LEFT:
		onGoalLeft();
		break;
	case FLG_GOAL_RIGHT:
		onGoalRight();
		break;
	case FLG_HANDBALL_LEFT:
		onHandballLeft();
		break;
	case FLG_HANDBALL_RIGHT:
		onHandballRight();
		break;
	case FLG_TIME_UP:
		onTimeUp();
		break;
	}
}

bool SoccerReferee::isBallHitGround()
{
	if (ballNode.translation[2] <= ballDiameter)	return true;
	return false;
}

//call this only once
void SoccerReferee::onStart()
{
	//start game timer

	//update display
	if (flag == FLG_PLAYER_READY)
	{
		gameMode = GM_BEFORE_KICK_OFF;
		flag = FLG_NONE;
	}
}

void SoccerReferee::onPlayerReady()
{
	if (lastGoalTeam == _PlayerNode::TEAM_LEFT)
		gameMode = GM_KICK_OFF_RIGHT;
	else
		gameMode = GM_KICK_OFF_LEFT;
	flag = FLG_NONE;
}

void SoccerReferee::onOutOfEndLineLeft()
{
	if (lastBallKeeperTeam == _PlayerNode::TEAM_LEFT)
		gameMode = GM_CORNER_KICK_LEFT;
	else
		gameMode = GM_GOAL_KICK_LEFT;
	flag = FLG_NONE;
}

void SoccerReferee::onOutOfEndLineRight()
{
	if (lastBallKeeperTeam == _PlayerNode::TEAM_LEFT)
		gameMode = GM_GOAL_KICK_RIGHT;
	else
		gameMode = GM_CORNER_KICK_RIGHT;
	flag = FLG_NONE;
}

void SoccerReferee::onOutOfSideLineLeft()
{
	
	gameMode = GM_THROW_IN_RIGHT;
	flag = FLG_NONE;
}

void SoccerReferee::onOutOfSideLineRight()
{

	gameMode = GM_THROW_IN_LEFT;
	flag = FLG_NONE;
}

void SoccerReferee::onOffsideLeft()
{
	flag = FLG_NONE;
}

void SoccerReferee::onOffsideRight()
{
	flag = FLG_NONE;
}

void SoccerReferee::onGoalLeft()
{
	score[0]++;
	gameMode = GM_KICK_OFF_RIGHT;
	flag = FLG_NONE;
}

void SoccerReferee::onGoalRight()
{
	score[1]++;
	gameMode = GM_KICK_OFF_LEFT;
	flag = FLG_NONE;
}

void SoccerReferee::onHandballLeft()
{
	flag = FLG_NONE;
}

void SoccerReferee::onHandballRight()
{
	flag = FLG_NONE;
}

void SoccerReferee::onTimeUp()
{
	gameMode = GM_GAME_OVER;
	flag = FLG_NONE;
}

void SoccerReferee::onComplete()
{
	gameMode = GM_PLAY_ON;
	flag = FLG_NONE;
}

int SoccerReferee::readHeader(std::string& src, std::string& dst, int offset)
{
	int start = 0;
	int end = 0;
	for (int i = offset; i < src.size(); i++)
	{
		if (src[i] == '(')
			start = i;
		else if (src[i] == ' ')
		{
			end = i;
			break;
		}
	}
	dst = src.substr(start, end - start);
	return end;
}

int SoccerReferee::readProperty(std::string& src, std::string& dst, int offset)
{
	int start = 0;
	int end = 0;

	for (int i = offset; i < src.size(); i++)
	{
		if (src[i] == '<')
			start = i;
		else if (src[i] == '>')
		{
			end = i;
			break;
		}
	}
	dst = src.substr(start, end - start);
	return end;
}

std::string SoccerReferee::seeBall(std::string& src)
{
	std::string msg = src + "(B (ccs " + 
		std::to_string(ballNode.translation[0]) + " " +
		std::to_string(ballNode.translation[1]) + " " +
		std::to_string(ballNode.translation[2]) + "))";
	return msg;
}

std::string SoccerReferee::seePlayer(std::string& src, int index)
{
	std::string msg = src + "(P (team " + mTeamName[vPlayerNodes[index].team] + ") (id " + 
		std::to_string(vPlayerNodes[index].id) + " ) (ccs " + 
		std::to_string(vPlayerNodes[index].translation[0]) + " " +
		std::to_string(vPlayerNodes[index].translation[1]) + " " +
		std::to_string(vPlayerNodes[index].translation[2]) + "))";
	return msg;
}



