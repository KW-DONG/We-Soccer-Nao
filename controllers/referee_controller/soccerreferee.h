#ifndef SOCCER_REFEREE_H
#define SOCCER_REFEREE_H

#include <webots/Supervisor.hpp>
#include <webots/Emitter.hpp>
#include <webots/Receiver.hpp>
#include <string>
#include <vector>
#include <atomic>

#define TIME_STEP 32

using namespace webots;

namespace webots {

class _Node
{	
public:
	void updatePosition();
	double translation[3] = {0,0,0};
	double rotation[3] = {0,0,0};
	Node* pNode = nullptr;
};

class SoccerReferee : public Supervisor
{

//events
enum {
	START,
	OUT_OF_ENDLINE_LEFT,
	OUT_OF_ENDLINE_RIGHT,
	OUT_OF_SIDELINE_LEFT,
	OUT_OF_SIDELINE_RIGHT,
	OFFSIDE_LEFT,
	OFFSIDE_RIGHT,
	GOAL_LEFT,
	GOAL_RIGHT,
	HANDBALL_LEFT,
	HANDBALL_RIGHT,
	HALF_TIME,
	TIME_UP,
	NONE
};

//play mode
enum {
	BEFORE_KICK_OFF,
	PLAY_ON,
	GAME_OVER,
	NONE,
	KICK_OFF_LEFT,
	KICK_OFF_RIGHT,
	THROW_IN_LEFT,
	THROW_IN_RIGHT,
	GOAL_KICK_LEFT,
	GOAL_KICK_RIGHT,
	FREE_KICK_LEFT,
	FREE_KICK_RIGHT,
	PENALTY_KICK_LEFT,
	PENALTY_KICK_RIGHT,
	CORNER_KICK_LEFT,
	CORNER_KICK_RIGHT
};


public:
	SoccerReferee();
	void run();

protected:
	void readConfig();
	void readPosition();

	/**
	* Protocal Format
	* (time (now 93.60)
	* (GS (t 0.00) (pm BeforeKickOff))
	* (See
	*	(B (ccs 0.0 1.0 0.05))
	*	(P (teamRed) (id 1) (ccs 0.0 0.0 0.5))
	*	(P (teamBlue) (id 2) (ccs 2.0 1.0 0.5)))
	*/
	void threadReceiver();
	void autoReferee();
private:
	std::vector<_Node> vPlayerLeftNodes;
	std::vector<_Node> vPlayerRightNodes;
	Emitter* pEmitter;
	Receiver* pReceiver;
	_Node ballNode;
	std::atomic<int> currentState;
	std::atomic<int> flag;
	int lastBallKeeper;
	bool isSocreValid;
	
	double pitchVec[2];
	double penaltyVec[2];
	double goalAreaVec[2];
	double goalVec[2];
	double goalZ;
	double penaltyMarkX;
	double penaltyMarkD;
	double centreD;
};

}


#endif