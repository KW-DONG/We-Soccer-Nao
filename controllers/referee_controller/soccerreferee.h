#ifndef SOCCER_REFEREE_H
#define SOCCER_REFEREE_H

#include <webots/Supervisor.hpp>
#include <webots/Emitter.hpp>
#include <webots/Receiver.hpp>
#include <string>
#include <vector>
#include <atomic>

using namespace webots;

namespace webots {

#define TIME_STEP 32
#define IS_LEFT_SEC(x) (x < 0)
#define IS_RIGHT_SEC(x) (x > 0)

//play mode
enum {
	GM_BEFORE_KICK_OFF,
	GM_PLAY_ON,
	GM_GAME_OVER,
	GM_NONE,
	GM_KICK_OFF_LEFT,
	GM_KICK_OFF_RIGHT,
	GM_THROW_IN_LEFT,
	GM_THROW_IN_RIGHT,
	GM_GOAL_KICK_LEFT,
	GM_GOAL_KICK_RIGHT,
	GM_FREE_KICK_LEFT,
	GM_FREE_KICK_RIGHT,
	GM_PENALTY_KICK_LEFT,
	GM_PENALTY_KICK_RIGHT,
	GM_CORNER_KICK_LEFT,
	GM_CORNER_KICK_RIGHT
};

//Pitch section
enum {
	PS_LEFT_GOAL,
	PS_LEFT_GOAL_AREA,
	PS_LEFT_PENALTY_AREA,
	PS_LEFT_NORMAL,
	PS_OUT_LEFT_ENDLINE,
	PS_OUT_RIGHT_ENDLINE,
	PS_OUT_SIDELINE,
	PS_RIGHT_GOAL,
	PS_RIGHT_GOAL_AREA,
	PS_RIGHT_PENALTY_AREA,
	PS_RIGHT_NORMAL,
};

class _Node
{	
public:
	void updatePosition();
	double translation[3] = { 0,0,0 };
	double rotation[4] = { 0,0,0,0 };
	double dTranslation[3] = { 0,0,0 };
	double lastTranslation[3] = { 0,0,0 };
	double lastRotation[4] = { 0,0,0,0 };
	double lastDTranslation[3] = { 0,0,0 };
	double velocity = 0.0;
	double lastVelocity = 0.0;
	
	Node* pNode = nullptr;
	int section;
	bool isReady;
};


class SoccerReferee : public Supervisor
{
  
enum {
	TEAM_LEFT,
	TEAM_RIGHT
};

public:
	SoccerReferee();

	void run();

	void findNodeSection(_Node& node);

	void findBallSection();

protected:
	void readPosition();

	void localReferee();

	void collisionDetection();

	bool isTeamLeft(int id);

	bool isTeamRight(int id);

	bool isGoalKeeper(int id);

	bool isBallHitGround();

	bool isBallInLeftHalf();

	bool isBallInRightHalf();

	void initPlayerPosition();

	void moveBall2D(double tx, double ty);

protected:
	void show();

	std::string seeBall(std::string& src);

	std::string seePlayer(std::string& src, int id);

private:
	std::vector<_Node> vPlayerNodes;
	//std::vector<_Node> vPlayerLeftNodes;
	//std::vector<_Node> vPlayerRightNodes;
	Emitter* pEmitter;
	Receiver* pReceiver;
	_Node ballNode;
	int gameMode = GM_NONE;
	int lastGameMode = GM_NONE;

	//last ball keeper detail
	//int lastBallKeeperTeam;
	int lastGoalTeam = -1;
	//int lastBallKeeperRole;
	int lastBallKeeperId = -1;
	double lastBallKeeperPosition[2];
	double lastBallInBoundsPosition[2];
	int lastBallKeeperSection;

	//time
	//std::string gameTime;
	int gameDuration;
	
	//pitch detail
	double pitchVec[2];
	double penaltyVec[2];
	double goalAreaVec[2];
	double goalVec[2];
	double goalZ;
	double penaltyMarkX;
	double ballDiameter;
	double robotZ;
	double centreD;

	//game detial
	int score[2] = {0,0};
	int teamPlayerNum;
	int totalPlayerNum;
};

}


#endif