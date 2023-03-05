#include "nao.h"
#include <Windows.h>

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
	"motion/HandWave.motion", "motions/Forwards.motion",
	"motion/TurnLeft60.motion", "motion/TurnRight60.motion",
	"motion/TurnLeft40.motion", "motion/TurnRight40.motion",
	"motion/TurnLeft180.motion","motions/Shoot.motion",
	"motion/StandUpFromFront.motion"
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
	error_state = false;
	error_id = -1;
}

void Nao::readPositionSensor()
{
	for (int i = 0; i < vPositionSensor.size(); i++)
	{
		vPositionSensorValue[i] = vPositionSensor[i]->getValue();
	}
}

bool Nao::move(double* target)
{
	//motion_stop();
	//stand();
	int times = 5;
	double target_2d[] = {target[0], target[1]};
	const double* in_position = pGPS->getValues();
	double cur_position[] = {in_position[0], in_position[1]};
	//pMotion[hand_wave]->setLoop(true);
	//play_syn(pMotion[hand_wave]);
	//std::cout << pMotion[hand_wave]->isValid() << std::endl;
	
	if (judge_position(target_2d, cur_position) > 0.2)
	{
		std::cout << judge_position(target_2d, cur_position) << std::endl;
		//motion_stop();
		double* rotation = (double*)pInertialUnit->getRollPitchYaw();
		double cur_rotation = rotation[2];
		//std::cout << sizeof(rotation) << std::endl;
		//double cur_rotation = pGyro->getValues()[2];
		double direction[] = {target[0] - cur_position[0], target[1] - cur_position[1]};
		double direct_angle = acos((direction[0]) / vector_length(direction));
		direct_angle = direction[1] > 0 ? std::abs(direct_angle) : -std::abs(direct_angle);
		//std::cout << direct_angle << std::endl;
		//std::cout <<  << std::endl;
		//std::cout << PI << std::endl;
		//if (cur_rotation > 0)
		//{
		//	if (direct_angle >= cur_rotation - PI && direct_angle <= cur_rotation)
		//	{
		//		if (play_syn(pMotion[turn_right_40]))
		//		{
		//			std::cout << "move1" << std::endl;
		//		}
		//		//current_motion = pMotion[turn_right_40];
		//	}
		//	else
		//	{
		//		play_syn(pMotion[turn_left_40]);
		//		//current_motion = pMotion[turn_left_40];
		//		std::cout << "move2" << std::endl;
		//	}
		//}
		//else
		//{
		//	if (direct_angle <= PI + cur_rotation && direct_angle >= cur_rotation)
		//	{
		//		play_syn(pMotion[turn_left_40]);
		//		//current_motion = pMotion[turn_left_40];
		//		std::cout << "move3" << std::endl;
		//	}
		//	else
		//	{
		//		play_syn(pMotion[turn_right_40]);
		//		//current_motion = pMotion[turn_right_40];
		//		std::cout << "move4" << std::endl;
		//	}
		//}
		//play_syn(pMotion[forwards]);
		//std::cout << ang_minus(cur_rotation, direct_angle) << std::endl;
		if (ang_minus(cur_rotation, direct_angle) > -(PI / 9) && ang_minus(cur_rotation, direct_angle) < (PI / 9))
		{
			play_syn(pMotion[forwards]);
			std::cout << "go straight" << std::endl;
		}
		else if (ang_minus(cur_rotation, direct_angle)<0)
		{
			play_syn(pMotion[turn_left_40]);
			std::cout << "turn left" << std::endl;
		}
		else if(ang_minus(cur_rotation, direct_angle) > 0)
		{
			play_syn(pMotion[turn_right_40]);
			std::cout << "turn right" << std::endl;
		}
		return true;
		//current_motion = pMotion[forwards];
		//pMotion[hand_wave]->setLoop(false);
		//motion_stop();
		/*in_position = pGPS->getValues();
		cur_position[0] = in_position[0];
		cur_position[1] = in_position[1];*/
	}
	//motion_stop();
	return false;
}

double Nao::judge_position(double* p1, double* p2)
{
	return sqrt(pow(p1[0] - p2[0], 2) + pow(p1[1] - p2[1], 2));
}

double Nao::vector_length(double v[])
{
	return sqrt(pow(v[0], 2) + pow(v[1], 2));
}

void Nao::motion_stop()
{
	//current_motion->stop();
	for (int i = 0; i < pMotion.size(); i++)
	{
		if (pMotion[i]->getTime()!=0.0)
		{
			pMotion[i]->stop();
			pMotion[i]->setTime(0);
		}
	}
	current_motion = NULL;
}

bool Nao::play_syn(Motion* mo)
{
	
	if (current_motion)
	{
		current_motion->stop();
		//current_motion = NULL;

	}
	mo->play();
	current_motion = mo;
	//mo->play();
	/*do
	{
		step(32);

	} while (!mo->isOver());*/
	std::cout << "play true" << std::endl;
	//current_motion = NULL;
	return true;
}

bool Nao::need_stand()
{
	//motion_stop();
	const double* acc = pAccelerometer->getValues();
	//std::cout << acc[0] << acc[1] << acc[2] << std::endl;
	/*if (current_motion == NULL || current_motion->isOver() && std::fabs(acc[0]) > std::fabs(acc[1]) &&
		std::fabs(acc[0]) > std::fabs(acc[2]) && acc[0] < -5)*/
	if (std::fabs(acc[0]) > std::fabs(acc[1]) && std::fabs(acc[0]) > std::fabs(acc[2]) && acc[0] < -5)
	{
		//motion_stop();
		//play_syn(pMotion[standup_fromfront]);
		//current_motion = pMotion[standup_fromfront];
		//current_motion->stop();
		//current_motion = NULL;
		error_state = true;
		error_id = stand;
		std::cout << "need true" << std::endl;
		return true;
	}
	std::cout << "need false" << std::endl;
	return false;
}

void Nao::do_the_correct(int number)
{
	/*if (current_motion != NULL)
	{
		current_motion->stop();
	}*/
	//std::cout << "correct2" << std::endl;
	
	if (error_id == stand)
	{
		//Sleep(8000);
		//std::cout << "correct3" << std::endl;
		//motion_stop();
		/*if (current_motion == NULL)
		{
			std::cout << "all right" << std::endl;
			current_motion = pMotion[standup_fromfront];
		}*/
		//pMotion[standup_fromfront]->play();
		/*if (current_motion == NULL)
		{
			play_syn(pMotion[standup_fromfront]);
			current_motion = pMotion[standup_fromfront];
		}*/
		if (pMotion[standup_fromfront]->isOver())
		{
			std::cout << "is standed" << std::endl;
			error_state = false;
			error_id = -1;
			pMotion[standup_fromfront]->setTime(0.0);
			return;
		}
		play_syn(pMotion[standup_fromfront]);
		/*if (pMotion[standup_fromfront]->isOver())
		{
			std::cout << "is standed" << std::endl;
			error_state = false;
			error_id = -1;
		}*/
		/*do
		{
			step(32);
			std::cout << "really did" << std::endl;
		} while (!pMotion[standup_fromfront]->isOver());*/
		//current_motion = NULL;
		//std::cout << "correct4" << std::endl;
		
	}
}

void Nao::play_seq(Motion* mo)
{
	
	if (error_state)
	{
		std::cout << "seq" << std::endl;
		do_the_correct(stand);
	}
	std::cout << "error_state" << error_state << std::endl;
	if (!error_state)
	{
		play_syn(mo);
	}
}

double Nao::change_angle(double ang)
{
	if (ang < 0)
	{
		return ang + PI;
	}
	else
		return ang;
}

double Nao::ang_minus(double ang1, double ang2)
{
	if (ang1 >= 0)
	{
		double sign = ang1 - PI;
		if (ang2 > ang1 || ang2 < sign)
		{
			return ang1 - change_angle(ang2);
		}
		else
			return ang1 - ang2;
	}
	else
	{
		double sign = PI + ang1;
		if (ang2<ang1 || ang2>sign)
		{
			return change_angle(ang1) - change_angle(ang2);
		}
		else
			return ang1 - ang2;
	}
}