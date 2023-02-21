#include "soccernao.h"

using namespace webots;

int main(int argc, char **argv) 
{
	SoccerNao* robot = new SoccerNao;
	robot->run();
	delete robot;
}
