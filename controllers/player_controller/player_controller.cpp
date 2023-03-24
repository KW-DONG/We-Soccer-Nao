#include "soccernao.h"
#include <Windows.h>

using namespace webots;

int main(int argc, char **argv) 
{
	SoccerNao* robot = new SoccerNao;
	
	robot->run();
	/*std::thread t1(&SoccerNao::thread_test, robot);
	std::thread t2(&SoccerNao::run, robot);
	t1.detach();
	t2.detach();*/
	delete robot;
}
