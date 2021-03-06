#ifndef INPUTS_H_
#define INPUTS_H_

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
#include "nrf_twi_mngr.h"
#include "nrf_drv_timer.h"

#include "buckler.h"
#include "display.h"
#include "kobukiActuator.h"
#include "kobukiSensorPoll.h"
#include "kobukiSensorTypes.h"
#include "kobukiUtilities.h"
#include "lsm9ds1.h"
#include "simple_ble.h"

#include "helpers.h"


// Input Struct
typedef struct inputs {
	float waypoint_distance; // in meters
	float waypoint_angle; // in degrees
	uint16_t right_encoder;
	uint16_t left_encoder;
	bool is_integrating;
	float gyro_integration_z_value;
	bool bump_left;
	bool bump_right;
	bool button_pressed;
	bool has_recently_connected;
	bool new_waypoint_written;
} inputs_t;


// Reads all sensors and aggregates any external data into a single struct
inputs_t get_inputs();

// Wrappers that start and stop the gyroscope integration
void stop_gyro_integration();
void start_gyro_integration();

#endif /* INPUTS_H_ */
