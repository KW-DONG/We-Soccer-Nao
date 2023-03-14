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


class SoccerNao : public Nao
{
public:
	SoccerNao();
	void run();
	void init(int number);
	bool receive_message();
	void read_message();
	void getPosition(std::string str, std::vector<double>& pos);
	void send_message(std::string header, std::string content);
	bool check_message(std::string message);
	void thread_test();
	void ready_to_play(int state);
	void move_with_avoid_target(double* target);//避障
	void getname_and_team();//获取名字和队伍
	void carry_ball(double* target);//运球
	void pass_ball(double* target);//传球

private:
	//std::thread receive;
	int player_number;
	std::vector<double> position;
	//std::vector<std::vector<double>> other_player;
	double** other_player;
	std::vector<double> rotation;
	std::vector<double> ball_position;
	double gametime;
	std::string gamemode;
	int team;
	int role;
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
};

}


#endif // !SOCCERNAO_H
