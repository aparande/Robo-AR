#ifndef STATES_H_
#define STATES_H_

#define TIME_MAX 20
#define AVOID_DIST_INCR 0.5
#define BACKWARD_DIST 0.3

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

#include "outputs.h"
#include "inputs.h"




typedef enum {
    OFF=0,
    WAITING=1,
    TURNING=2,
    DRIVING=3,
    END_TURNING=4,
} states;

typedef enum {
	FORWARD=0,
	STOPPED=1,
	BACKWARD=2,
	ROTATING=3,
	AVOIDANCE=4,
} substates;


typedef enum {
	NO_BUMP=0,
	LEFT_BUMP=1,
	RIGHT_BUMP=2
} bumps;

typedef struct driving_substate {
	substates substate;
	uint16_t previous_left_encoder;
	uint16_t previous_right_encoder;
	float total_distance_traveled_left;
	float total_distance_traveled_right;
	float turn_angle_substate;
	float relative_orientation_angle;
	substates next_state_turning;
	uint8_t stopping_timer;
	bumps most_recent_bump;
	float avoidance_distance;
  float target_forward_distance;

} driving_substate_t;

typedef struct system_state {
	states state;
	driving_substate_t substate;
	float position_x;
	float position_y;
	// float curr_waypoint_distance;
	// float curr_waypoint_angle;
	float curr_orientation_angle;
	float turn_angle;
	float distance_to_travel;
	int acknowledged_val;
} system_state_t;




void transition_in(inputs_t input_state, system_state_t* curr_state);
void transition_out(inputs_t input_state, system_state_t* curr_state, states old_state);

void substate_transition_in(inputs_t input_state, driving_substate_t* curr_state);
void substate_transition_out(inputs_t input_state, driving_substate_t* curr_state, substates old_state);

void print_state(system_state_t current_state, char* display_line_0, char* display_line_1);
void print_substate(system_state_t current_state, char* display_line0, char* display_line_1);

outputs_t transition(inputs_t input_state, system_state_t* curr_state);
outputs_t substate_transition(inputs_t input_state, system_state_t* curr_state);

system_state_t init_state();
driving_substate_t init_substate();





#endif /* STATES_H_ */
