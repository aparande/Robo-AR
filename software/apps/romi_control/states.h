#ifndef STATES_H_
#define STATES_H_

#define TIME_MAX 20
#define AVOID_DIST_INCR 0.5
#define BACKWARD_DIST -0.3
#define AVOID_ANGLE 45
#define K_DIST 210
#define K_DIFF 250
#define K_TURN .8
#define MIN_ANGLE_SPEED 50
#define MIN_DRIVE_SPEED 80
#define ANGLE_THRESHOLD .5
#define DISTANCE_THRESHOLD .02

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


// Main FSM States
typedef enum {
    OFF=0, // Phone is disconnected from the Robot
    WAITING=1, // Robot is waiting for a waypoint
    TURNING=2, // Robot is turning before driving
    DRIVING=3, // Robot is driving with obstacle avoidance
    END_TURNING=4, // Robot is turning back to its original position
} states;

//Substate States
typedef enum {
	FORWARD=0, // Robot is driving forward
	STOPPED=1, // Robot has hit an obstacle
	BACKWARD=2, // Robot is moving backward to avoid obstacle
	ROTATING=3, // Robot is rotating to avoid the obstacle
	AVOIDANCE=4, // Robot is driving away from the obstacle
} substates;

// Front Bumper Sides
typedef enum {
	NO_BUMP=0,
	LEFT_BUMP=1,
	RIGHT_BUMP=2
} bumps;

// Variables that describe the state of the robot during DRIVING state
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

// Variables that describe the state of the overall Robot. 
typedef struct system_state {
	states state;
	driving_substate_t substate;
	float position_x;
	float position_y;
	float curr_orientation_angle;
	float turn_angle;
	float distance_to_travel;
	int acknowledged_val;
} system_state_t;


// Transition Functions
outputs_t transition(inputs_t input_state, system_state_t* curr_state);
outputs_t substate_transition(inputs_t input_state, system_state_t* curr_state);


// Transition Function Helpers

// Function is called whenever a transition from one state to another happens in the main FSM
// It deals with any startup behavior involved in transitioning INTO a given state
void transition_in(inputs_t input_state, system_state_t* curr_state); 

// Function is called whenever a transition from one state to another happens in the main FSM
// Deals with any cleanup behavior involved in transitioning OUT OF a state
void transition_out(inputs_t input_state, system_state_t* curr_state, states old_state); //

// Function is called whenever a transition from one state to another happens in the child FSM
// It deals with any startup behavior involved in transitioning INTO a given state
void substate_transition_in(inputs_t input_state, driving_substate_t* curr_state);

// Function is called whenever a transition from one state to another happens in the child FSM
// Deals with any cleanup behavior involved in transitioning OUT OF a state
void substate_transition_out(inputs_t input_state, driving_substate_t* curr_state, substates old_state);

// Printing Functions. Determines what is written on the display. 
void print_state(system_state_t current_state, char* display_line_0, char* display_line_1);
void print_substate(system_state_t current_state, char* display_line0, char* display_line_1);

// Control functions. Given reference value, compute motor inputs and update some state values.
void turning_controls(float target_angle, inputs_t input_state, system_state_t* curr_state, outputs_t* output);
void driving_controls(float target_distance, inputs_t input_state, system_state_t* curr_state, outputs_t* output);

// State Initializers
system_state_t init_state();
driving_substate_t init_substate();

#endif /* STATES_H_ */
