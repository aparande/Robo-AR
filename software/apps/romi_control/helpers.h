#ifndef HELPERS_H_
#define HELPERS_H_

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>


#include "app_error.h"
#include "app_timer.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_spi.h"

#include "buckler.h"
#include "display.h"
#include "kobukiActuator.h"
#include "kobukiSensorPoll.h"
#include "kobukiSensorTypes.h"
#include "kobukiUtilities.h"
#include "lsm9ds1.h"
#include "simple_ble.h"


// Global Variables 

KobukiSensors_t sensors;
ret_code_t error_code;


// Intervals for advertising and connections
simple_ble_config_t ble_config;

//4607eda0-f65e-4d59-a9ff-84420d87a4ca
simple_ble_service_t robot_service;

//Declare characteristics and variables for your service
simple_ble_char_t waypoint_char;
simple_ble_char_t ack_char;
simple_ble_app_t* simple_ble_app;

const float CONVERSION;
const float angle_threshold;
const float distance_threshold;
int k_dist;
int k_diff;
uint16_t min_angle_speed;
uint16_t min_drive_speed;

float waypoint[2];
bool new_waypoint_written;
int acknowledged;
bool connected;



float measure_distance(uint16_t current_encoder, uint16_t previous_encoder);

void setup();


#endif /* HELPERS_H_ */