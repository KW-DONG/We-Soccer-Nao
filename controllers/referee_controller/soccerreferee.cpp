#include "soccerreferee.h"
#include <cstdlib>
#include <iostream>
#include <map>
#include "global.h"
#include <opencv.hpp>

using namespace webots;

#define CONFIG_PATH		"../config.ini"
#define ALWAYS_PLAY_ON	1

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

void _Node::updatePosition()
{
	memcpy(translation, pNode->getField("translation")->getSFVec3f(), sizeof(translation));
	memcpy(rotation, pNode->getField("rotation")->getSFRotation(), sizeof(rotation));
}

SoccerReferee::SoccerReferee()
{	
	std::cout << "start referee" << std::endl;
	//soccer pitch
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

	//game
	GetIniKeyString("Game", "PlayerNum", CONFIG_PATH, value);
	teamPlayerNum = atoi(value);
	totalPlayerNum = teamPlayerNum * 2;

	GetIniKeyString("Game", "DurationSec", CONFIG_PATH, value);
	gameDuration = atoi(value);
	
	//team
	static _PlayerNode _node;
	_node.isReady = false;
	_node.role = _PlayerNode::PLAYER;
	for (int i = 0; i < teamPlayerNum; i++)
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
		//setSFVec3f
		double startPoint[3] = {-penaltyVec[0], penaltyVec[1] - i, 0.4};
		double startRot[4] = { 0,0,1,0 };
		_node.pNode->getField("translation")->setSFVec3f(startPoint);
		_node.pNode->getField("rotation")->setSFRotation(startRot);
		vPlayerNodes.push_back(_node);
	}
	vPlayerNodes[vPlayerNodes.size()].role = _PlayerNode::GOAL_KEEPER;

	for (int i = 0; i < teamPlayerNum; i++)
	{
		std::string key = "DEF" + std::to_string(i);
		GetIniKeyString("Team2", key.c_str(), CONFIG_PATH, value);
		_node.pNode = getFromDef(value);
		if (_node.pNode == NULL)
		{
			std::cerr << "No DEF " << value << " node found in the current world file" << std::endl;
			exit(1);
		}
		_node.id = i + teamPlayerNum;
		_node.team = _PlayerNode::TEAM_RIGHT;
		double startPoint[3] = { penaltyVec[0], penaltyVec[1] - i, 0.4 };
		double startRot[4] = { 0,0,1,3.14 };
		_node.pNode->getField("translation")->setSFVec3f(startPoint);
		_node.pNode->getField("rotation")->setSFRotation(startRot);
		vPlayerNodes.push_back(_node);
	}
	
	//ball
	GetIniKeyString("Ball", "Diameter", CONFIG_PATH, value);
	ballDiameter = atof(value);

	GetIniKeyString("Ball", "DEF", CONFIG_PATH, value);
	ballNode.pNode = getFromDef(value);
	if (ballNode.pNode == NULL)
	{
		std::cerr << "No DEF " << value << " node found in the current world file" << std::endl;
		exit(1);
	}
	double startPoint[3] = { 0, 0, ballDiameter };
	ballNode.pNode->getField("translation")->setSFVec3f(startPoint);

	pEmitter = getEmitter("emitter");
	pReceiver = getReceiver("receiver");
	pReceiver->enable(TIME_STEP);

#if (ALWAYS_PLAY_ON)
	gameMode = GM_PLAY_ON;
#endif
}

void SoccerReferee::run()
{
	while (step(TIME_STEP) != -1)
	{		
		readReceiver();
		readPosition();	

#if (!ALWAYS_PLAY_ON)
		if (flag != FLG_HANDBALL_LEFT && flag != FLG_HANDBALL_RIGHT && flag != FLG_TIME_UP)
			localReferee();
		lastGameMode = gameMode;

		stateDriver();

		//boardcast playmode
		if (lastGameMode != gameMode)
#endif
		{
			std::string msg = "(GS (time " + std::to_string(getTime()) + ") (pm " + mPlayMode[gameMode] + "))";
			pEmitter->setChannel(-1);
			pEmitter->send(msg.c_str(), msg.size());
			std::cout << msg << std::endl;
		}
		
		//send position
		{
			std::string msg = "(See (";
			msg = seeBall(msg);
			for (int i = 0; i < totalPlayerNum; i++)
			{
				msg = seePlayer(msg, i);
			}
			msg += "))";
			pEmitter->send(msg.c_str(), msg.size());
			std::cout << msg << std::endl;
		}
		
		show();
		if (gameDuration < getTime())
			flag = FLG_TIME_UP;
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
		std::string sHeader;
		int offset = readHeader(data, sHeader, 0);

		lastBallKeeperId = readId(data, offset);

		if (sHeader == "RD")
		{
			vPlayerNodes[lastBallKeeperId].isReady = true;
		}
		else
		{
			if (lastBallKeeperId < teamPlayerNum)
				lastBallKeeperTeam = _PlayerNode::TEAM_LEFT;		
			else				
				lastBallKeeperTeam = _PlayerNode::TEAM_RIGHT;

			memcpy(lastBallKeeperPosition, vPlayerNodes[lastBallKeeperId].translation, 2);
			lastBallKeeperRole = vPlayerNodes[lastBallKeeperId].role;
		
			if (sHeader == "HB")
			{
				if (lastBallKeeperTeam == _PlayerNode::TEAM_LEFT)
				{
					if (lastBallKeeperRole == _PlayerNode::GOAL_KEEPER)
						flag = FLG_KICKBALL_LEFT;
					else
						flag = FLG_HANDBALL_LEFT;
				}
				else
				{
					if (lastBallKeeperRole == _PlayerNode::GOAL_KEEPER)
						flag = FLG_KICKBALL_RIGHT;
					else
						flag = FLG_HANDBALL_RIGHT;
				}
			}
			else if (sHeader == "KB")
			{
				if (lastBallKeeperRole == _PlayerNode::GOAL_KEEPER)
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
	double _point[3] = { 0.0, 0.0, ballDiameter };
	ballNode.pNode->getField("translation")->setSFVec3f(_point);
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
	double _point[3] = { 0.0, 0.0, ballDiameter };
	ballNode.pNode->getField("translation")->setSFVec3f(_point);
}

void SoccerReferee::onGoalRight()
{
	score[1]++;
	gameMode = GM_KICK_OFF_LEFT;
	flag = FLG_NONE;
	double _point[3] = { 0.0, 0.0, ballDiameter };
	ballNode.pNode->getField("translation")->setSFVec3f(_point);
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

int SoccerReferee::readId(std::string& src, int offset)
{
	size_t start = src.find("id ", offset) + 3;
	size_t end = src.find(" )", start);
	
	return atoi(src.substr(start, end - start).c_str());
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
	std::string msg = src + "(P (id " + 
		std::to_string(vPlayerNodes[index].id) + ") (ccs " + 
		std::to_string(vPlayerNodes[index].translation[0]) + " " +
		std::to_string(vPlayerNodes[index].translation[1]) + " " +
		std::to_string(vPlayerNodes[index].translation[2]) + "))";
	return msg;
}

void paintPitchArea(cv::Mat& mat, double* areaVec, double* pitchVec, double scale, int offsetX, int offsetY)
{
	cv::line(mat,
		cv::Point(areaVec[0] * scale + offsetX, areaVec[1] * scale + offsetY),
		cv::Point(areaVec[0] * scale + offsetX, -areaVec[1] * scale + offsetY),
		cv::Scalar(0, 0, 0));
	cv::line(mat,
		cv::Point(-areaVec[0] * scale + offsetX, areaVec[1] * scale + offsetY),
		cv::Point(-areaVec[0] * scale + offsetX, -areaVec[1] * scale + offsetY),
		cv::Scalar(0, 0, 0));
	cv::line(mat,
		cv::Point(areaVec[0] * scale + offsetX, areaVec[1] * scale + offsetY),
		cv::Point(pitchVec[0] * scale + offsetX, areaVec[1] * scale + offsetY),
		cv::Scalar(0, 0, 0));
	cv::line(mat,
		cv::Point(areaVec[0] * scale + offsetX, -areaVec[1] * scale + offsetY),
		cv::Point(pitchVec[0] * scale + offsetX, -areaVec[1] * scale + offsetY),
		cv::Scalar(0, 0, 0));
	cv::line(mat,
		cv::Point(-areaVec[0] * scale + offsetX, areaVec[1] * scale + offsetY),
		cv::Point(-pitchVec[0] * scale + offsetX, areaVec[1] * scale + offsetY),
		cv::Scalar(0, 0, 0));
	cv::line(mat,
		cv::Point(-areaVec[0] * scale + offsetX, -areaVec[1] * scale + offsetY),
		cv::Point(-pitchVec[0] * scale + offsetX, -areaVec[1] * scale + offsetY),
		cv::Scalar(0, 0, 0));
}

void SoccerReferee::show()
{
	cv::Mat mat(480, 720, CV_8UC3, cv::Scalar(255,255,255));
	
	double scale = 50.0;
	int offsetX = 360;
	int offsetY = 240;

	//pitch
	cv::line(mat,
		cv::Point(pitchVec[0] * scale + offsetX, pitchVec[1] * scale + offsetY),
		cv::Point(-pitchVec[0] * scale + offsetX, pitchVec[1] * scale + offsetY),
		cv::Scalar(0, 0, 0));
	cv::line(mat,
		cv::Point(-pitchVec[0] * scale + offsetX, pitchVec[1] * scale + offsetY),
		cv::Point(-pitchVec[0] * scale + offsetX, -pitchVec[1] * scale + offsetY),
		cv::Scalar(0, 0, 0));
	cv::line(mat,
		cv::Point(-pitchVec[0] * scale + offsetX, -pitchVec[1] * scale + offsetY),
		cv::Point(pitchVec[0] * scale + offsetX, -pitchVec[1] * scale + offsetY),
		cv::Scalar(0, 0, 0));
	cv::line(mat,
		cv::Point(pitchVec[0] * scale + offsetX, -pitchVec[1] * scale + offsetY),
		cv::Point(pitchVec[0] * scale + offsetX, pitchVec[1] * scale + offsetY),
		cv::Scalar(0, 0, 0));

	//middle line
	cv::line(mat,
		cv::Point(offsetX, pitchVec[1] * scale + offsetY),
		cv::Point(offsetX, -pitchVec[1] * scale + offsetY),
		cv::Scalar(0, 0, 0));

	//circle
	cv::circle(mat, cv::Point(offsetX, offsetY), centreD * scale * 0.5, cv::Scalar(0, 0, 0));

	//penalty area
	paintPitchArea(mat, penaltyVec, pitchVec, scale, offsetX, offsetY);
	
	//goal area
	paintPitchArea(mat, goalAreaVec, pitchVec, scale, offsetX, offsetY);

	//goal
	paintPitchArea(mat, goalVec, pitchVec, scale, offsetX, offsetY);

	//text
	std::string _score = "Score " + std::to_string(score[0]) + " : " + std::to_string(score[1]);
	std::string _gameState = "Game State: " + mPlayMode[gameMode];
	std::string _time = "Time: " + std::to_string(getTime()) + "s";
	cv::putText(mat, _score, cv::Point(50, 20), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 0));
	cv::putText(mat, _gameState, cv::Point(50, 50), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 0));
	cv::putText(mat, _time, cv::Point(200, 20), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 0));

	//ball
	cv::circle(mat, cv::Point(ballNode.translation[0] * scale + offsetX,
		-ballNode.translation[1] * scale + offsetY), 5,
		cv::Scalar(128, 128, ballNode.translation[2] * 50));

	//player
	for (int i = 0; i < vPlayerNodes.size(); i++)
	{
		cv::Scalar color;
		if (vPlayerNodes[i].team == _PlayerNode::TEAM_LEFT)
			color = cv::Scalar(255, 0, 0);
		else
			color = cv::Scalar(0, 0, 255);

		cv::circle(mat, cv::Point(vPlayerNodes[i].translation[0] * scale + offsetX,
			-vPlayerNodes[i].translation[1] * scale + offsetY), 7, color);
	}

	cv::imshow("Display", mat);
	cv::waitKey(15);
}
