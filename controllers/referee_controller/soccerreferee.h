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

class _Node
{	
public:
	void updatePosition();
	double translation[3] = {0,0,0};
	double rotation[4] = {0,0,0,0};
	Node* pNode = nullptr;
	int section;
	bool isReady;
};

class _PlayerNode : public _Node
{
public:
	enum {
		TEAM_LEFT,
		TEAM_RIGHT
	};
	int team;
	enum {
		PLAYER,
		GOAL_KEEPER
	};
	int role;
	int id;
};

class SoccerReferee : public Supervisor
{
//player id
enum {
	PLAYER_LEFT_0,
	PLAYER_LEFT_1,
	PLAYER_LEFT_2,
	GOAL_KEEPER_LEFT,
	PLAYER_RIGHT_0,
	PLAYER_RIGHT_1,
	PLAYER_RIGHT_2,
	GOAL_KEEPER_RIGHT
};

//events
enum {
	FLG_START,
	FLG_PLAYER_READY,
	FLG_OUT_OF_ENDLINE_LEFT,
	FLG_OUT_OF_ENDLINE_RIGHT,
	FLG_OUT_OF_SIDELINE_LEFT,
	FLG_OUT_OF_SIDELINE_RIGHT,
	FLG_OFFSIDE_LEFT,
	FLG_OFFSIDE_RIGHT,
	FLG_GOAL_LEFT,
	FLG_GOAL_RIGHT,
	FLG_HANDBALL_LEFT,
	FLG_HANDBALL_RIGHT,
	FLG_KICKBALL_LEFT,
	FLG_KICKBALL_RIGHT,
	FLG_BALL_HIT_GROUND,
	FLG_HALF_TIME,
	FLG_TIME_UP,
	FLG_NONE,
	FLG_COMPLETE
};

//Pitch section
enum {
	PS_LEFT_GOAL = -4,
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

public:
	SoccerReferee();
	void run();
	void findNodeSection(_Node& node);
	void findBallSection();

protected:
	void readPosition();
	void readReceiver();
	void localReferee();
	void stateDriver();
	bool isBallHitGround();

protected:
	//callbacks
	void onStart();
	void onPlayerReady();
	void onOutOfEndLineLeft();
	void onOutOfEndLineRight();
	void onOutOfSideLineLeft();
	void onOutOfSideLineRight();
	void onOffsideLeft();
	void onOffsideRight();
	void onGoalLeft();
	void onGoalRight();
	void onHandballLeft();
	void onHandballRight();	
	void onTimeUp();
	void onComplete();

	int readHeader(std::string& src, std::string& dst, int offset=0);
	int readId(std::string& src, int offset);

	void show();

	std::string seeBall(std::string& src);
	std::string seePlayer(std::string& src, int id);

private:
	std::vector<_PlayerNode> vPlayerNodes;
	//std::vector<_Node> vPlayerLeftNodes;
	//std::vector<_Node> vPlayerRightNodes;
	Emitter* pEmitter;
	Receiver* pReceiver;
	_Node ballNode;
	int gameMode = GM_NONE;
	int lastGameMode = GM_NONE;
	std::atomic<int> flag = FLG_NONE;

	//last ball keeper detail
	int lastBallKeeperTeam;
	int lastGoalTeam;
	int lastBallKeeperRole;
	int lastBallKeeperId;
	double lastBallKeeperPosition[2];

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
	double centreD;

	//game detial
	int score[2] = {0,0};
	int teamPlayerNum;
	int totalPlayerNum;

};

}


#endif