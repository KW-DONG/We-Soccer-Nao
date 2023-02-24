#include "nao.h"

const std::vector<std::string> vPositionSensorName = {
	"HeadPitchS", "HeadYawS",
	"LAnklePitchS", "LAnkleRollS",
	"LElbowRollS", "LElbowYawS",
	"LHipPitchS", "LHipRollS", "LHipYawPitchS",
	"LKneePitchS",
	"LShoulderPitchS", "LShoulderRollS",
	"LWristYawS",
	"RAnklePitchS", "RAnkleRollS",
	"RElbowRollS", "RElbowYawS",
	"RHipPitchS", "RHipRollS", "RHipYawPitchS",
	"RKneePitchS",
	"RShoulderPitchS", "RShoulderRollS",
	"RWristYawS"
};

const std::vector<std::string> vMotorName = {
	"HeadPitch", "HeadYaw",
	"LAnklePitch", "LAnkleRoll",
	"LElbowRoll", "LElbowYaw",
	"LHipPitch", "LHipRoll", "LHipYawPitch",
	"LKneePitch",
	"LShoulderPitch", "LShoulderRoll",
	"LWristYaw",
	"RAnklePitch", "RAnkleRoll",
	"RElbowRoll", "RElbowYaw",
	"RHipPitch", "RHipRoll", "RHipYawPitch",
	"RKneePitch",
	"RShoulderPitch", "RShoulderRoll",
	"RWristYaw",
};

const std::string accelerometerName = "accelerometer";

const std::vector<std::string> vCameraName = {
	"CameraBottom", "CameraTop"
};

const std::vector<std::string> vEmitterName = {
	"emitter", "super_emitter"
};

const std::string gpsName = "gps";

const std::string gyroName = "gyro";

const std::string inertialUnitName = "inertial unit";

const std::vector<std::string> vTouchSensorName = {
	"LFoot/Bumper/Left", "LFoot/Bumper/Right", "LFsr",
	"RFoot/Bumper/Left", "RFoot/Bumper/Right", "RFsr"
};

const std::string receiverName = "receiver";

const std::vector<std::string> vDistanceSensorName = {
	"Sonar/Left", "Sonar/Right"
};

const std::vector<std::string> vMotionName = {
	"motions/HandWave.motion", "motions/Forwards.motion",
	"motions/Backwards.motion", "motions/SideStepLeft.motion",
	"motions/SideStepRight.motion", "motions/TurnLeft60.motion",
	"motions/TurnRight60.motion"
};

Nao::Nao()
{
	for (int i = 0; i < vPositionSensorName.size(); i++)
	{
		PositionSensor* pSensor = getPositionSensor(vPositionSensorName[i]);
		pSensor->enable(TIME_STEP);
		vPositionSensor.push_back(pSensor);
		vPositionSensorValue.push_back(0.0);
	}
		
	for (int i = 0; i < vMotorName.size(); i++)
	{
		Motor* pMotor = getMotor(vMotorName[i]);
		vMotor.push_back(pMotor);
	}

	pAccelerometer = getAccelerometer(accelerometerName);
	pAccelerometer->enable(TIME_STEP);

	for (int i = 0; i < vCameraName.size(); i++)
	{
		Camera* pCamera = getCamera(vCameraName[i]);
		pCamera->enable(TIME_STEP);
		vCamera.push_back(pCamera);
	}

	for (int i = 0; i < vEmitter.size(); i++)
	{
		Emitter* pEmitter = getEmitter(vEmitterName[i]);
		vEmitter.push_back(pEmitter);
	}
	
	pGPS = getGPS(gpsName);
	pGPS->enable(TIME_STEP);

	pGyro = getGyro(gyroName);
	pGyro->enable(TIME_STEP);

	pInertialUnit = getInertialUnit(inertialUnitName);
	pInertialUnit->enable(TIME_STEP);

	for (int i = 0; i < vTouchSensorName.size(); i++)
	{
		TouchSensor* pTouchSensor = getTouchSensor(vTouchSensorName[i]);
		pTouchSensor->enable(TIME_STEP);
		vTouchSensor.push_back(pTouchSensor);
	}

	pReceiver = getReceiver(receiverName);
	pReceiver->enable(TIME_STEP);

	for (int i = 0; i < vDistanceSensorName.size(); i++)
	{
		DistanceSensor* pDistanceSensor = getDistanceSensor(vDistanceSensorName[i]);
		pDistanceSensor->enable(TIME_STEP);
		vDistanceSensor.push_back(pDistanceSensor);
	}

	for (int i = 0; i < vMotionName.size(); i++)
	{
		Motion* pmotion = new Motion(vMotionName[i]);
		pMotion.push_back(pmotion);
	}

	/*Motion hand_wave("../../motions/HandWave.motion");
	Motion forwards("../../motions/Forwards.motion");
	Motion backwards("../../motions/Backwards.motion");
	Motion side_step_left("../../motions/SideStepLeft.motion");
	Motion side_step_right("../../motions/SideStepRight.motion");
	Motion turn_left_60("../../motions/TurnLeft60.motion");
	Motion turn_right_60("../../motions/TurnRight60.motion");*/
	current_motion = NULL;
}

void Nao::readPositionSensor()
{
	for (int i = 0; i < vPositionSensor.size(); i++)
	{
		vPositionSensorValue[i] = vPositionSensor[i]->getValue();
	}
}

void Nao::move(double* target)
{
	motion_stop();
	int times = 5;
	double target_2d[] = {target[0], target[1]};
	const double* in_position = pGPS->getValues();
	double cur_position[] = {in_position[0], in_position[1]};
	//pMotion[hand_wave]->setLoop(true);
	//play_syn(pMotion[hand_wave]);
	//std::cout << pMotion[hand_wave]->isValid() << std::endl;
	
	if (judge_position(target_2d, cur_position) > 0.5)
	{
		
		//motion_stop();
		double* rotation = (double*)pInertialUnit->getRollPitchYaw();
		double cur_rotation = rotation[2];
		//std::cout << sizeof(rotation) << std::endl;
		//double cur_rotation = pGyro->getValues()[2];
		double direction[] = {target[0] - cur_position[0], target[1] - cur_position[1]};
		double direct_angle = acos((direction[0]) / vector_length(direction));
		direct_angle = direction[1] > 0 ? std::abs(direct_angle) : -std::abs(direct_angle);
		std::cout << direct_angle << std::endl;
		//std::cout <<  << std::endl;
		//std::cout << PI << std::endl;
		if (cur_rotation > 0)
		{
			if (direct_angle >= cur_rotation - PI && direct_angle <= cur_rotation)
			{
				play_syn(pMotion[turn_right_60]);
				std::cout << "move1" << std::endl;
			}
			else
			{
				play_syn(pMotion[turn_left_60]);
				std::cout << "move2" << std::endl;
			}
		}
		else
		{
			if (direct_angle <= PI + cur_rotation && direct_angle >= cur_rotation)
			{
				play_syn(pMotion[turn_left_60]);
				std::cout << "move3" << std::endl;
			}
			else
			{
				play_syn(pMotion[turn_right_60]);
				std::cout << "move4" << std::endl;
			}
		}
		play_syn(pMotion[forwards]);
		//pMotion[hand_wave]->setLoop(false);
		//motion_stop();
		/*in_position = pGPS->getValues();
		cur_position[0] = in_position[0];
		cur_position[1] = in_position[1];*/
	}
	motion_stop();
}

double Nao::judge_position(double* p1, double* p2)
{
	return sqrt(pow(p1[0] - p2[0], 2) + pow(p1[1] - p2[1], 2) + pow(p1[2] - p2[2], 2));
}

double Nao::vector_length(double v[])
{
	return sqrt(pow(v[0], 2) + pow(v[1], 2));
}

void Nao::motion_stop()
{
	for (int i = 0; i < pMotion.size(); i++)
	{
		pMotion[i]->stop();
	}
}

void Nao::play_syn(Motion* mo)
{
	mo->play();
	do
	{
		step(TIME_STEP);
	} while (!mo->isOver());
}
