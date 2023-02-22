#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <webots/accelerometer.h>
#include <webots/camera.h>
#include <webots/distance_sensor.h>
#include <webots/gps.h>
#include <webots/gyro.h>
#include <webots/inertial_unit.h>
#include <webots/keyboard.h>
#include <webots/led.h>
#include <webots/motor.h>
#include <webots/robot.h>
#include <webots/touch_sensor.h>
#include <webots/utils/motion.h>

static void start_motion(WbMotionRef motion);