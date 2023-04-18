#ifndef SOCCERNAO_H
#define SOCCERNAO_H
#include "nao.h"
#include <thread>
#include <map>
#include <regex>
#include <map>
#include <vector>
#include <atomic>
#include <queue>

using namespace webots;

namespace webots {

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

	enum 
	{
		Move,
		Pass_Ball,
		Stand,
		Kick_Ball
	};

	enum
	{
		Normal,
		Ready_to_Pass,
		Ready_to_Kick,
		Error
	};

	struct Action {
		double* _ball_position;
		double** _other_player;
		int action_type;
	};

	struct PlayerInfo {
		double dist2ToBall;
		int action;
		double actionParam[2];
	};

class SoccerNao : public Nao
{
public:
	SoccerNao();
	void run();
	bool receive_message();
	void read_message();

	//parse position information from string
	void getPosition(std::string str, double* pos);
	void sendMessage(std::string header, std::string content);
	bool check_message(std::string message);

	//void moveObstacleAvoidance(double* target);//避障
	//void getname_and_team();//获取名字和队伍
	void soccerPaser();

	//check player id
	bool isStriker(int id);
	bool isDefender(int id);
	bool isGoalKeeper(int id);
	bool isTeamLeft(int id);
	bool isTeamRight(int id);
	bool isSameTeam(int id);
	bool isKickOffTeam(int id);
	bool isKickOffDefenderTeam(int id);
	bool isThrowInTeam(int id);
	bool isThrowInDefenderTeam(int id);
	bool isCornerKickTeam(int id);
	bool isCornerKickDefenderTeam(int id);
	bool isClosest2Ball(int id);
	bool isClosest2BallTeam(int id);
	bool isClosest2BallDefender(int id);
	bool isPlayerReach(int id, Vec2D vec);
	bool isTeamGotBall(int id);
	bool isBallInPenaltyArea(int id);

	//strategy
	bool canShoot(int id);
	bool canPassBall(int fId, int tId);
	void getBestShootPlace(int id, Vec2D vec);
	void getBestShootVec(int id, Vec2D vec);
	void getBestPassBallPlace(int fId, Vec2D fVec, int tId, Vec2D tVec);
	void getBestKickOffVec(int id, Vec2D vec);
	void getBestThrowInVec(int id, Vec2D vec);
	void getBestCornerKickVec(int id, Vec2D vec);

	void getDefenderAttackPosition(int id, Vec2D pos);
	
	int getClosest2BallDefender();
	int getClosest2BallStriker();
	int getClosestStrikerId(int id);
	int getClosestDefenderId(int id);


	void setPlayerAction(int id, int action);
	void setPlayerActionParam(int id, Vec2D param);

private:
	//std::thread receive;
	int player_number;
	std::vector<double> position;

	//std::vector<std::vector<double>> other_player;
	double** other_player;
	std::vector<double> rotation;
	std::vector<double> ball_position;
	double gametime;
	int gameMode;
	//int team;
	//int role;
	std::queue<std::string> messages;
	std::queue<Action*> actions;
	int player_state;
	Emitter* pemitter;
	Receiver* preceiver;

protected:
	/**
	* Protocal
	* (time (now 93.60)	//system time
	* (GS (t 0.00) (pm BeforeKickOff))	//game time and game mode
	* (See
	*	(B (ccs 0.0 1.0 0.05))	//ball
	*	(P (teamRed) (id 1) (ccs 0.0 0.0 0.5))	//player
	*	(P (teamBlue) (id 2) (ccs 2.0 1.0 0.5)))
	*/

private:
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

	int teamPlayerNum;
	int playerId;
	int playerTeam;
	//int playerRole;

	//action
	int action;
	double actionParam[2];

	//positions for all player from both teams
	std::vector<std::vector<double>> playerPositions;
	
	//the ball position
	double ballPosition[3];

	//player action info for all player
	std::vector<PlayerInfo> vPlayerInfo;

	//a sorted playerId vector, lower index means closer to the ball;
	std::vector<int> vDist2Rnk;
	std::vector<int> vDist2RnkTeam;

};

}


#endif // !SOCCERNAO_H
