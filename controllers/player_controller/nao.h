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
	hand_wave, forwards, backwards, side_step_left, side_step_right,
	turn_left_60, turn_right_60
};

public:
	Nao();
	void move(double* target);
	double judge_position(double* p1, double* p2);
	double vector_length(double v[]);
	void motion_stop();

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
