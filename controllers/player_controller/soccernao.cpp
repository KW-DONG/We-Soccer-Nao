#include "soccernao.h"
#include <Windows.h>

using namespace webots;

SoccerNao::SoccerNao()
{

}

void SoccerNao::run()
{
	while (step(TIME_STEP) != -1)
	{
		
	}
}

void ScooerNao::init(int number)
{
	player_number = number;
}