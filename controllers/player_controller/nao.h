#ifndef NAO_H
#define NAO_H
#include <webots/Robot.hpp>
#include <webots/Motor.hpp>
#include <webots/Accelerometer.hpp>
#include <webots/Camera.hpp>
#include <webots/Emitter.hpp>
#include <webots/GPS.hpp>
#include <webots/Gyro.hpp>
#include <webots/PositionSensor.hpp>
#include <webots/InertialUnit.hpp>
#include <webots/TouchSensor.hpp>
#include <webots/Receiver.hpp>
#include <webots/DistanceSensor.hpp>
#include <webots/utils/motion.h>
#include <webots/utils/Motion.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>

#define TIME_STEP 32
#define PI acos(-1.0)

using namespace webots;

typedef double* Vec2D;
typedef double* Vec3D;

namespace webots {

class Nao : public Robot
{
enum {
	HeadPitch, HeadYaw,
	LAnklePitch, LAnkleRoll,
	LElbowRoll, LElbowYaw,
	LHipPitch, LHipRoll, LHipYawPitch,
	LKneePitch,
	LShoulderPitch, LShoulderRoll,
	LWristYaw,
	RAnklePitch, RAnkleRoll,
	RElbowRoll, RElbowYaw,
	RHipPitch, RHipRoll, RHipYawPitch,
	RKneePitch,
	RShoulderPitch, RShoulderRoll,
	RWristYaw
};

enum {
	CameraBottom, CameraTop
};

enum {
	emitter, super_emitter
};

enum {
	LBumpLeft, LBumpRight, LFsr,
	RBumpLeft, RBumpRight, RFsr
};

enum {
	LSonar, RSonar
};

enum {
	forwards, turn_left_60, turn_right_60, turn_left_40, turn_right_40,
	turn_left_180, shoot, standup_fromfront, turn_left_20, turn_right_20,
	side_step_right
};

enum {
	stand
};

public:
	Nao();
	bool move(double* target);
	double judge_position(double* p1, double* p2);
	double vector_length(double v[]);
	void motion_stop();
	bool play_syn(Motion* mo);
	bool need_stand();
	void do_the_correct(int number);
	double change_angle(double ang);
	double ang_minus(double ang1, double ang2);
	bool kick_towards(double* target, double* ballposition);
	int change_direction(double* target);
	bool is_between_2_point(double* p1, double* p2);
	//bool g02(double* centre, double radius, double rad);//˳ʱ��
	bool g03(double* centre, double radius, double rad);//��ʱ��
	bool move_with_avoid(int playerId, double* target, std::vector<std::vector<double>>& playerPosition, bool crossBall=false, double* ballposition=nullptr);
	int is_on_the_way(double* bposition, double* target);

protected:
	void readPositionSensor();
	std::vector<Emitter*> vEmitter;
	Receiver* pReceiver;
	std::vector<PositionSensor*> vPositionSensor;
	std::vector<double> vPositionSensorValue;
	std::vector<Motor*> vMotor;
	Accelerometer* pAccelerometer;
	std::vector<Camera*> vCamera;
	GPS* pGPS;
	Gyro* pGyro;
	InertialUnit* pInertialUnit;
	std::vector<TouchSensor*> vTouchSensor;
	std::vector<DistanceSensor*> vDistanceSensor;
	std::vector<Motion*> pMotion;
	int error_id;
	bool error_state;
	bool dirty_kick_toward;
	bool dirty_kick_2;
	bool dirtu_circle_1;
	bool dirty_move_avoid;
	//Motion hand_wave;
	//Motion forwards;
	//Motion backwards;
	//Motion side_step_left;
	//Motion side_step_right;
	//Motion turn_left_60;
	//Motion turn_right_60;
	/*Motion hand_wave{ "../../motions/HandWave.motion" };
	Motion forwards{ "../../motions/Forwards.motion" };
	Motion backwards{ "../../motions/Backwards.motion" };
	Motion side_step_left{ "../../motions/SideStepLeft.motion" };
	Motion side_step_right{ "../../motions/SideStepRight.motion" };
	Motion turn_left_60{ "../../motions/TurnLeft60.motion" };
	Motion turn_right_60{ "../../motions/TurnRight60.motion" };*/
	Motion* current_motion;

private:

};

}



#endif // !NAO_H
