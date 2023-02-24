#include "soccernao.h"
#include <Windows.h>

using namespace webots;

int main(int argc, char **argv) 
{
	SoccerNao* robot = new SoccerNao;
	//std::thread t1(&SoccerNao::get_message, robot);
	//t1.detach();
	robot->run();
	
	delete robot;
}
