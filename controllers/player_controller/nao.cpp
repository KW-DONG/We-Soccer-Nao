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
}

void Nao::readPositionSensor()
{
	for (int i = 0; i < vPositionSensor.size(); i++)
	{
		vPositionSensorValue[i] = vPositionSensor[i]->getValue();
	}
}
