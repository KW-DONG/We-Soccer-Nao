#include "soccerreferee.h"
#include <cstdlib>
#include <iostream>
#include <map>
#include "global.h"
#include <opencv.hpp>

using namespace webots;

#define CONFIG_PATH		"../config.ini"
#define ALWAYS_PLAY_ON	0

#define _SQ(X) (X)*(X)

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

std::map<int, std::string> mPitchSection = {
	{PS_LEFT_GOAL, "LeftGoal"},
	{PS_LEFT_GOAL_AREA, "LeftGoalArea"},
	{PS_LEFT_PENALTY_AREA, "LeftPenaltyArea"},
	{PS_LEFT_NORMAL, "LeftNormal"},
	{PS_OUT_LEFT_ENDLINE, "OutLeftEndline"},
	{PS_OUT_RIGHT_ENDLINE, "OutRightEndline"},
	{PS_OUT_SIDELINE, "OutSideLine"},
	{PS_RIGHT_GOAL, "RightGoal"},
	{PS_RIGHT_GOAL_AREA, "RightGoalArea"},
	{PS_RIGHT_PENALTY_AREA, "RightPenaltyArea"},
	{PS_RIGHT_NORMAL, "RightNormal"}
};

void _Node::updatePosition()
{
	memcpy(lastRotation, rotation, sizeof(rotation));
	memcpy(lastTranslation, translation, sizeof(translation));
	memcpy(lastDTranslation, dTranslation, sizeof(dTranslation));
	lastVelocity = velocity;
	memcpy(translation, pNode->getField("translation")->getSFVec3f(), sizeof(translation));
	memcpy(rotation, pNode->getField("rotation")->getSFRotation(), sizeof(rotation));
	for (int i = 0; i < 3; i++)
		dTranslation[i] = translation[i] - dTranslation[i];
	velocity = sqrt(_SQ(dTranslation[0]) + _SQ(dTranslation[1]) + _SQ(dTranslation[2]));
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
	static _Node _node;
	_node.isReady = false;
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
		vPlayerNodes.push_back(_node);
	}

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
		vPlayerNodes.push_back(_node);
	}

	GetIniKeyString("Robot", "Z", CONFIG_PATH, value);
	robotZ = atof(value);
	
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
}

void SoccerReferee::run()
{
	gameMode = GM_BEFORE_KICK_OFF;

	while (step(TIME_STEP) != -1)
	{				
		readPosition();
		findBallSection();
		collisionDetection();
		localReferee();

		//boardcast playmode
		if (lastGameMode != gameMode)
		{
			lastGameMode = gameMode;
			std::string msg = "(GS (time " + std::to_string(getTime()) + ") (pm " + mPlayMode[gameMode] + "))";
			pEmitter->setChannel(-1);
			pEmitter->send(msg.c_str(), msg.size()+1);
		}

		//send position
		{
			std::string msg = "(See (";
			msg = seeBall(msg);
			for (int i = 0; i < totalPlayerNum; i++)
				msg = seePlayer(msg, i);
			
			msg += "))";
			pEmitter->send(msg.c_str(), msg.size()+1);
		}
		
		show();
	}
}

void SoccerReferee::findNodeSection(_Node& node)
{
	if (node.translation[0] <= -penaltyVec[0] &&
		node.translation[1] <= penaltyVec[1] &&
		node.translation[1] >= -penaltyVec[1])
	{
		if (node.translation[0] <= -goalAreaVec[0] &&
			node.translation[1] <= goalAreaVec[1] &&
			node.translation[1] >= -goalAreaVec[1])
			node.section = PS_LEFT_GOAL_AREA;
		else node.section = PS_LEFT_PENALTY_AREA;
	}
	else if (node.translation[0] >= penaltyVec[0] &&
		node.translation[1] <= penaltyVec[1] &&
		node.translation[1] >= -penaltyVec[1])
	{
		if (node.translation[0] >= goalAreaVec[0] &&
			node.translation[1] <= goalAreaVec[1] &&
			node.translation[1] >= -goalAreaVec[1])
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
	else if (ballNode.translation[0] > pitchVec[0] && ballNode.translation[0] > -goalVec[0] &&
		ballNode.translation[1] < goalVec[1] && ballNode.translation[1] > -goalVec[1] &&
		ballNode.translation[2] < goalZ)
		ballNode.section = PS_RIGHT_GOAL;
	//ball hit ground
	else if (isBallHitGround())
	{
		if (ballNode.translation[0] < -pitchVec[0])
			ballNode.section = PS_OUT_LEFT_ENDLINE;
		else if (ballNode.translation[0] > pitchVec[0])
			ballNode.section = PS_OUT_RIGHT_ENDLINE;
		else if (ballNode.translation[1] < -pitchVec[1] || ballNode.translation[1] > pitchVec[1])
			ballNode.section = PS_OUT_SIDELINE;
		else
		{
			findNodeSection(ballNode);
			lastBallInBoundsPosition[0] = ballNode.translation[0];
			lastBallInBoundsPosition[1] = ballNode.translation[1];
		}
		//std::cout << "hit ground " << ballNode.translation[0] << " " << ballNode.translation[1] << " " << mPitchSection[ballNode.section] << std::endl;
	}
	else
	{
		findNodeSection(ballNode);
		lastBallInBoundsPosition[0] = ballNode.translation[0];
		lastBallInBoundsPosition[1] = ballNode.translation[1];
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

void SoccerReferee::collisionDetection()
{
	//is ball hit something
	if (ballNode.velocity > 0.001 &&
		(ballNode.velocity > ballNode.lastVelocity * 1.2 ||
			ballNode.velocity < ballNode.lastVelocity * 0.8))
	{
		double minDist2 = 0.06;	
		//find which robot is the cloest to the ball
		int id = -1;
		for (int i = 0; i < totalPlayerNum; i++)
		{
			double dx = vPlayerNodes[i].translation[0] - ballNode.translation[0];
			double dy = vPlayerNodes[i].translation[1] - ballNode.translation[1];

			double dist2 = _SQ(dx) + _SQ(dy);
			if (dist2 < minDist2)
			{
				minDist2 = dist2;
				id = i;
			}
		}
		if (id != -1)
		{
			lastBallKeeperId = id;
			lastBallKeeperPosition[0] = vPlayerNodes[id].translation[0];
			lastBallKeeperPosition[1] = vPlayerNodes[id].translation[1];
			lastBallKeeperSection = vPlayerNodes[id].section;
		}
	}
}

void SoccerReferee::localReferee()
{			
	if (getTime() >= gameDuration)
	{
		gameMode = GM_GAME_OVER;
	}
	else if (gameMode == GM_BEFORE_KICK_OFF)
	{
		moveBall2D(0, 0);
		initPlayerPosition();

		if (lastGoalTeam == TEAM_LEFT)
			gameMode = GM_KICK_OFF_RIGHT;
		else
			gameMode = GM_KICK_OFF_LEFT;

		lastBallKeeperId = -1;
	}
	else if (gameMode == GM_KICK_OFF_LEFT)
	{
		if (lastBallKeeperId >= 0 && lastBallKeeperId < teamPlayerNum)
			gameMode = GM_PLAY_ON;
	}
	else if (gameMode == GM_KICK_OFF_RIGHT)
	{
		if (lastBallKeeperId >= teamPlayerNum && lastBallKeeperId < totalPlayerNum)
			gameMode = GM_PLAY_ON;
	}
	else if (gameMode == GM_PLAY_ON)
	{
		if (ballNode.section == PS_LEFT_GOAL)
		{
			score[1]++;
			lastGoalTeam = TEAM_RIGHT;
			gameMode = GM_BEFORE_KICK_OFF;
		}
		else if (ballNode.section == PS_RIGHT_GOAL)
		{
			score[0]++;
			lastGoalTeam = TEAM_LEFT;
			gameMode = GM_BEFORE_KICK_OFF;
		}
		else if (ballNode.section == PS_OUT_SIDELINE)
		{
			moveBall2D(lastBallInBoundsPosition[0], lastBallInBoundsPosition[1]);
			lastBallKeeperId = -1;
			if (lastBallKeeperId < teamPlayerNum)
				gameMode = GM_THROW_IN_RIGHT;
			else
				gameMode = GM_THROW_IN_LEFT;
		}
		else if (ballNode.section == PS_OUT_LEFT_ENDLINE)
		{
			lastBallKeeperId = -1;
			if (isTeamLeft(lastBallKeeperId))
			{
				//corner kick right
				if (lastBallInBoundsPosition[1] > 0)
					moveBall2D(-pitchVec[0], pitchVec[1]);
				else
					moveBall2D(-pitchVec[0], -pitchVec[1]);
				gameMode = GM_CORNER_KICK_RIGHT;
			}
			else if (isTeamRight(lastBallKeeperId))
			{
				//goal kick left
				moveBall2D(-(goalAreaVec[0] + penaltyVec[0]) * 0.5, 0);
				gameMode = GM_GOAL_KICK_LEFT;
			}
		}
		else if (ballNode.section == PS_OUT_RIGHT_ENDLINE)
		{
			lastBallKeeperId = -1;
			if (lastBallKeeperId < teamPlayerNum)
			{
				//corner kick left
				if (lastBallInBoundsPosition[1] > 0)
					moveBall2D(pitchVec[0], pitchVec[1]);
				else
					moveBall2D(pitchVec[0], -pitchVec[1]);
				gameMode = GM_CORNER_KICK_LEFT;
			}
			else
			{
				//goal kick right
				moveBall2D((goalAreaVec[0] + penaltyVec[0]) * 0.5, 0);
				gameMode = GM_GOAL_KICK_RIGHT;
			}
		}
		else if (isBallInLeftHalf())
		{
			
		}
		else if (isBallInRightHalf())
		{

		}
	}
	else if (gameMode == GM_THROW_IN_LEFT)
	{
		if (isTeamLeft(lastBallKeeperId))
			gameMode = GM_PLAY_ON;
	}
	else if (gameMode == GM_THROW_IN_RIGHT)
	{
		if (isTeamRight(lastBallKeeperId))
			gameMode = GM_PLAY_ON;
	}
	else if (gameMode == GM_GOAL_KICK_LEFT)
	{
		if (isGoalKeeper(lastBallKeeperId) && isTeamLeft(lastBallKeeperId))
			gameMode = GM_PLAY_ON;
	}
	else if (gameMode == GM_GOAL_KICK_RIGHT)
	{
		if (isGoalKeeper(lastBallKeeperId) && isTeamRight(lastBallKeeperId))
			gameMode = GM_PLAY_ON;
	}
	else if (gameMode == GM_CORNER_KICK_LEFT)
	{
		if (isTeamLeft(lastBallKeeperId))
			gameMode = GM_PLAY_ON;
	}
	else if (gameMode == GM_CORNER_KICK_RIGHT)
	{
		if (isTeamRight(lastBallKeeperId))
			gameMode = GM_PLAY_ON;
	}
}

bool SoccerReferee::isTeamLeft(int id)
{
	if (id >= 0 && id < teamPlayerNum)
		return true;
	return false;
}

bool SoccerReferee::isTeamRight(int id)
{
	if (id >= teamPlayerNum && id < totalPlayerNum)
		return true;
	return false;
}

bool SoccerReferee::isGoalKeeper(int id)
{
	if (id == teamPlayerNum - 1 || id == totalPlayerNum - 1)
		return true;
	return false;
}

bool SoccerReferee::isBallHitGround()
{
	if (ballNode.translation[2] <= ballDiameter)	return true;
	return false;
}

bool SoccerReferee::isBallInLeftHalf()
{
	if (ballNode.section > PS_LEFT_GOAL || ballNode.section <= PS_LEFT_NORMAL)
		return true;
	return false;
}

bool SoccerReferee::isBallInRightHalf()
{
	if (ballNode.section > PS_RIGHT_GOAL || ballNode.section <= PS_RIGHT_NORMAL)
		return true;
	return false;
}

void SoccerReferee::initPlayerPosition()
{	
	for (int i = 0; i < teamPlayerNum; i++)
	{
		const double startRot[4] = { 0,0,1,0 };
		double startPoint[3];
			
		//goal keeper
		if (i == teamPlayerNum - 1)
		{
			startPoint[0] = -penaltyVec[0];
			startPoint[1] = 0;
			startPoint[2] = robotZ;
		}
		//striker
		else if (i == 0)
		{
			startPoint[0] = -centreD * 0.5;
			startPoint[1] = 0;
			startPoint[2] = robotZ;
		}
		else if (i == 1)
		{
			startPoint[0] = -centreD * 0.5;
			startPoint[1] = 2;
			startPoint[2] = robotZ;
		}
		else if (i == 2)
		{
			startPoint[0] = -centreD * 0.5;
			startPoint[1] = -2;
			startPoint[2] = robotZ;
		}

		vPlayerNodes[i].pNode->getField("rotation")->setSFRotation(startRot);
		vPlayerNodes[i].pNode->getField("translation")->setSFVec3f(startPoint);
	}

	for (int i = teamPlayerNum; i < totalPlayerNum; i++)
	{
		const double startRot[4] = { 0,0,1,3.14 };
		double startPoint[3];

		//goal keeper
		if (i == totalPlayerNum - 1)
		{
			startPoint[0] = penaltyVec[0];
			startPoint[1] = 0;
			startPoint[2] = robotZ;
		}
		//striker
		else if (i == teamPlayerNum) 
		{
			startPoint[0] = centreD * 0.5;
			startPoint[1] = 0;
			startPoint[2] = robotZ;
		}
		else if (i == teamPlayerNum + 1)
		{
			startPoint[0] = centreD * 0.5;
			startPoint[1] = 2;
			startPoint[2] = robotZ;
		}
		else if (i == teamPlayerNum + 2)
		{
			startPoint[0] = centreD * 0.5;
			startPoint[1] = -2;
			startPoint[2] = robotZ;
		}
		vPlayerNodes[i].pNode->getField("rotation")->setSFRotation(startRot);
		vPlayerNodes[i].pNode->getField("translation")->setSFVec3f(startPoint);
	}
}

void SoccerReferee::moveBall2D(double tx, double ty)
{
	double _point[3] = { tx, ty, ballDiameter };
	ballNode.pNode->getField("translation")->setSFVec3f(_point);
	double _rotation[4] = { 0,0,0,0 };
	ballNode.pNode->getField("rotation")->setSFRotation(_rotation);
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
		std::to_string(index) + ") (ccs " + 
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
	std::string _lastGoalTeam = "Last Goal Team: " + std::to_string(lastGoalTeam);
	std::string _lastBallKeeper = "Last Ball Keeper: " + std::to_string(lastBallKeeperId);
	//std::string _ballSection = "Ball Section: " + mPitchSection[ballNode.section];

	cv::putText(mat, _score, cv::Point(50, 20), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 0));
	cv::putText(mat, _gameState, cv::Point(50, 50), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 0));
	cv::putText(mat, _time, cv::Point(200, 20), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 0));
	cv::putText(mat, _lastGoalTeam, cv::Point(400, 20), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 0));
	cv::putText(mat, _lastBallKeeper, cv::Point(400, 50), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 0));
	//cv::putText(mat, _ballSection, cv::Point(400, 80), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0, 0, 0));

	//ball
	cv::circle(mat, cv::Point(ballNode.translation[0] * scale + offsetX,
		-ballNode.translation[1] * scale + offsetY), 5,
		cv::Scalar(128, 128, ballNode.translation[2] * 50));

	//player
	for (int i = 0; i < vPlayerNodes.size(); i++)
	{
		cv::Scalar color;
		if (isTeamLeft(i))
			color = cv::Scalar(255, 0, 0);
		else if (isTeamRight(i))
			color = cv::Scalar(0, 0, 255);

		cv::circle(mat, cv::Point(vPlayerNodes[i].translation[0] * scale + offsetX,
			-vPlayerNodes[i].translation[1] * scale + offsetY), 7, color);
	}

	cv::imshow("Display", mat);
	cv::waitKey(50);
}
