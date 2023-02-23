#include "soccernao.h"

using namespace webots;

int main(int argc, char **argv) 
{
	SoccerNao* robot = new SoccerNao;
	//std::thread t1(&SoccerNao::receive_message, robot);
	
	robot->run();
	//t1.join();
	delete robot;
}
