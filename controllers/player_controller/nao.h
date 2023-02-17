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
#include <iostream>
#include <vector>
#include <string>

#define TIME_STEP 32

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

public:
	Nao();

protected:
	void readPositionSensor();
	std::vector<Emitter*> vEmitter;

private:
	std::vector<PositionSensor*> vPositionSensor;
	std::vector<double> vPositionSensorValue;
	Receiver* pReceiver;

	std::vector<Motor*> vMotor;
	Accelerometer* pAccelerometer;
	std::vector<Camera*> vCamera;
	GPS* pGPS;
	Gyro* pGyro;
	InertialUnit* pInertialUnit;
	std::vector<TouchSensor*> vTouchSensor;
	std::vector<DistanceSensor*> vDistanceSensor;
};

}



#endif // !NAO_H
