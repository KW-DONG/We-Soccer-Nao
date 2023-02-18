#ifndef SOCCERNAO_H
#define SOCCERNAO_H
#include "nao.h"
#include <thread>
#include <map>
#include <regex>
#include <map>
#include <vector>

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

class SoccerNao : public Nao
{
public:
	SoccerNao();
	void run();
	void init(int number);
	void receive_message();
	void getpositions(std::string str, std::vector<double>& pos);
private:
	std::thread receive;
	int player_number;
	std::vector<double> position;
	double gametime;
	std::string gamemode;
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
};

}


#endif // !SOCCERNAO_H
