#include "soccerreferee.h"
#include <cstdlib>
#include <thread>
#include <ctime>
#include <cstdlib>
#include <map>

using namespace webots;

#define BALL_DIAMETER 0.05

const std::vector<std::string> vPlayerLeftDef = {
	"R0", "R1", "R2", "RG"
};

const std::vector<std::string> vPlayerRightDef = {
	"B0", "B1", "B2", "BG"
};

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

void SoccerReferee::readConfig()
{
	
}

SoccerReferee::SoccerReferee()
{	
	for (int i = 0; i < vPlayerLeftDef.size(); i++)
	{
		_PlayerNode _node;
		_node.pNode = getFromDef(vPlayerLeftDef[i]);
		if (_node.pNode == NULL)
		{
			std::cerr << "No DEF " << vPlayerLeftDef[i] << " node found in the current world file" << std::endl;
			exit(1);
		}
		_node.id = i;
		_node.role = _PlayerNode::PLAYER;
		_node.team = _PlayerNode::TEAM_LEFT;
		vPlayerNodes.push_back(_node);
	}
	vPlayerNodes[vPlayerNodes.size() - 1].role = _PlayerNode::GOAL_KEEPER;

	for (int i = 0; i < vPlayerRightDef.size(); i++)
	{
		_PlayerNode _node;
		_node.pNode = getFromDef(vPlayerRightDef[i]);
		if (_node.pNode == NULL)
		{
			std::cerr << "No DEF " << vPlayerRightDef[i] << " node found in the current world file" << std::endl;
			exit(1);
		}
		_node.id = i;
		_node.role = _PlayerNode::PLAYER;
		_node.team = _PlayerNode::TEAM_RIGHT;
		vPlayerNodes.push_back(_node);
	}
	vPlayerNodes[vPlayerNodes.size() - 1].role = _PlayerNode::GOAL_KEEPER;

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
		for (int i = 0; i < vPlayerNodes.size(); i++)
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
	else if (ballNode.translation[2] <= BALL_DIAMETER)
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

		if (sTeam == "left")
		{
			lastBallKeeperPosition[0] = vPlayerNodes[lastBallKeeperId].translation[0];
			lastBallKeeperPosition[1] = vPlayerNodes[lastBallKeeperId].translation[1];
			lastBallKeeperRole = vPlayerNodes[lastBallKeeperId].role;
			lastBallKeeperTeam = _PlayerNode::TEAM_LEFT;
		}		
		else if (sTeam == "right")
		{
			lastBallKeeperPosition[0] = vPlayerNodes[lastBallKeeperId + 4].translation[0];
			lastBallKeeperPosition[1] = vPlayerNodes[lastBallKeeperId + 4].translation[1];
			lastBallKeeperRole = vPlayerNodes[lastBallKeeperId + 4].role;
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
			else if (sTeam == "RIGHT")
			{
				if (lastBallKeeperRole == _PlayerNode::GOAL_KEEPER)
					flag = FLG_KICKBALL_RIGHT;
				else
					flag = FLG_HANDBALL_RIGHT;
			}
		}
		else if (sHeader == "KB")
		{
			if (sTeam == "LEFT")
				flag = FLG_KICKBALL_LEFT;
			else
				flag = FLG_KICKBALL_RIGHT;
		}
		pReceiver->nextPacket();
	}
}

void SoccerReferee::localReferee()
{		
	if (gameMode == GM_PLAY_ON)
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
	if (ballNode.translation[2] <= BALL_DIAMETER)	return true;
	return false;
}

void SoccerReferee::onStart()
{
	//start game timer

	//update display

	
	gameMode = GM_BEFORE_KICK_OFF;
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

int SoccerReferee::readHeader(std::string& src, std::string& dst, int offset=0)
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



