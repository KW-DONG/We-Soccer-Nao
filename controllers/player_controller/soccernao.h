#ifndef SOCCERNAO_H
#define SOCCERNAO_H
#include "nao.h"
#include <thread>
#include <map>

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
	int readHeader(std::string& src, std::string& dst, int offset);
	int readProperty(std::string& src, std::string& dst, int offset);
private:
	std::thread receive;
	int player_number;

};

}


#endif // !SOCCERNAO_H
