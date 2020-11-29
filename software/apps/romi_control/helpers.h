

/*
 * states.h
 *
 *  Created on: Sep 22, 2018
 *      Author: shromonaghosh
 */

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

#include "states.h"



// Global Variables 

KobukiSensors_t sensors = {0};
ret_code_t error_code = NRF_SUCCESS;


// Intervals for advertising and connections
static simple_ble_config_t ble_config = {
        // c0:98:e5:49:xx:xx
        .platform_id       = 0x46,    // used as 4th octect in device BLE address
        .device_id         = 0xEEC5, 
        .adv_name          = "Robo-AR", // used in advertisements if there is room
        .adv_interval      = MSEC_TO_UNITS(1000, UNIT_0_625_MS),
        .min_conn_interval = MSEC_TO_UNITS(100, UNIT_1_25_MS),
        .max_conn_interval = MSEC_TO_UNITS(200, UNIT_1_25_MS),
};

//4607eda0-f65e-4d59-a9ff-84420d87a4ca
static simple_ble_service_t robot_service = {{
    .uuid128 = {0xca,0xa4,0x87,0x0d,0x42,0x84,0xff,0xA9,
                0x59,0x4D,0x5e,0xf6,0xa0,0xed,0x07,0x46}
}};

//Declare characteristics and variables for your service
static simple_ble_char_t waypoint_char = {.uuid16 = 0xeda1};
static simple_ble_char_t ack_char = {.uuid16 = 0xeda2};
simple_ble_app_t* simple_ble_app;

const float CONVERSION = 0.0006108;
const float angle_threshold = .5;
const float distance_threshold = .02;
int k_dist = 190;
int k_diff = 250;


float waypoint[2] = {0, 0};
bool new_waypoint_written = false;
int acknowledged = -1;
bool connected = false;

void ble_evt_connected(ble_evt_t const* p_ble_evt) {
    connected = true;
}

void ble_evt_disconnected(ble_evt_t const* p_ble_evt) {
    connected = false;
}

// void read_input() {
//     printf("Bluetooth message recieved\n");
//     if (state == WAITING) {
//         printf("Distance: %f\n", waypoint[0]);
//         printf("Angle: %f\n", waypoint[1]);
//     }
// }

void ble_evt_write(ble_evt_t const* p_ble_evt) {
    //logic for each characteristic and related state changes
    //Try not to modify the state here...
    new_waypoint_written = true;
}

void setup();



#endif /* HELPERS_H_ */