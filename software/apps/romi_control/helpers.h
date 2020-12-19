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

#define CONVERSION 0.0006108


// Global Variables 
KobukiSensors_t sensors;
ret_code_t error_code;
bool is_gyro_integrating;


// Bluetooth 

// Intervals for advertising and connections
simple_ble_config_t ble_config;

//4607eda0-f65e-4d59-a9ff-84420d87a4ca
simple_ble_service_t robot_service;

//Declare characteristics and variables for your service
simple_ble_char_t waypoint_char;
simple_ble_char_t ack_char;
simple_ble_app_t* simple_ble_app;

//BLE global variables
float waypoint[2];
bool new_waypoint_written;
int acknowledged;
bool connected;


// Util Functions

// Measures the distance in meters given the encoder values of the current and previous timestep. 
float measure_distance(uint16_t current_encoder, uint16_t previous_encoder);

// Modulo function that takes any givne float and places the float between -180 and 180 degrees. 
float angle_modulo(float possible_angle);

// Setup Functions
void setup();


#endif /* HELPERS_H_ */