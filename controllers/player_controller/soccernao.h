#ifndef SOCCERNAO_H
#define SOCCERNAO_H
#include "nao.h"
#include <thread>

using namespace webots;

namespace webots {

class SoccerNao : public Nao
{
public:
	SoccerNao();
	void run();
	void init();
private:
	std::thread receive;
	int player_number;

};

}


#endif // !SOCCERNAO_H
