#include <webots/Supervisor.hpp>
#include <string>
#include <vector>

using namespace webots;

namespace webots {

class SoccerReferee : public Supervisor
{
public:
	SoccerReferee();
	void run();

private:
	std::vector<Node*> vPlayerNodes1;
	std::vector<Node*> vPlayerNodes2;
	Node* pBallNode;

};

}