#include "states.h"


system_state_t init_state() {
	system_state_t ret_system = {0};
	ret_system.acknowledged_val = 0;
	ret_system.state = OFF;
	return ret_system;

}

void transition_in(inputs_t input_state, system_state_t* curr_state) {
	switch(curr_state->state) {
		case OFF: {
			(*curr_state) = init_state();
			break;
		}
		case WAITING: {
			*curr_state = (system_state_t) {0};
			curr_state->state = WAITING;
			break;
		}
		case TURNING: {
			curr_state->curr_orientation_angle = 0;
			stop_gyro_integration();
			start_gyro_integration();
			break;
		}
		case DRIVING: {

			// Initializes the hierarchical state machine. Localizing itself at 0.
			curr_state->substate = init_substate();
			curr_state->curr_orientation_angle = 0;
			curr_state->position_x = 0;
			curr_state->position_y = 0;
			curr_state->substate.previous_left_encoder = input_state.left_encoder;
			curr_state->substate.previous_right_encoder = input_state.right_encoder;
      		curr_state->substate.target_forward_distance = curr_state->distance_to_travel;
			break;
		}
		case END_TURNING: {
			curr_state->curr_orientation_angle = 0;
			stop_gyro_integration();
			start_gyro_integration();
			break;
		}
	}
}

void transition_out(inputs_t input_state, system_state_t* curr_state, states old_state) {
	switch(old_state) {
		case OFF: {
			break;
		}
		case WAITING: {
			break;
		}
		case TURNING: {
			stop_gyro_integration();
			curr_state->turn_angle = 0;
			curr_state->curr_orientation_angle = 0;
			break;
		}
		case DRIVING: {
			stop_gyro_integration();
			// resets driving state
			curr_state->substate = init_substate();
			break;
		}
		case END_TURNING: {
			stop_gyro_integration();
			curr_state->turn_angle = 0;
			curr_state->curr_orientation_angle = 0;
			break;
		}
	}
}

outputs_t transition(inputs_t input_state, system_state_t* curr_state) {
	outputs_t output = {0};
	states old_state = curr_state->state;
	switch(curr_state->state) {
		case OFF: {
			if (input_state.has_recently_connected) {
          		curr_state->state = WAITING;
				break;
        	} else {
          		output.left_speed = 0;
          		output.right_speed = 0;
          		curr_state->state = OFF;
				break;
        	}
		}
		case WAITING: {
	        if (input_state.button_pressed || !input_state.has_recently_connected) {
	          	curr_state->state = OFF;
	        } else if (input_state.new_waypoint_written) {
	        	curr_state->acknowledged_val = 1;
	        	output.notify_ack = true;
	        	curr_state->turn_angle = input_state.waypoint_angle;
	        	curr_state->distance_to_travel = input_state.waypoint_distance;
            curr_state->state = TURNING;
	        } else {
				output.left_speed = 0;
				output.right_speed = 0;
		        curr_state->state = WAITING;
	        }
	        break;
	    }
      	case TURNING: {
      		curr_state->curr_orientation_angle = input_state.gyro_integration_z_value; 
	        float diff = angle_modulo(curr_state->turn_angle - curr_state->curr_orientation_angle);
	        if (input_state.button_pressed || !input_state.has_recently_connected) {
	          	curr_state->state = OFF;
	        } else if (fabs(diff) < angle_threshold) {
	            curr_state->state = DRIVING;
	        }
	        else {
	          	int8_t sign = (2 * (diff > 0)) - 1;
	          	int16_t speed = sign * fmax(.8 * fabs(diff), min_angle_speed);
				output.left_speed = -speed;
				output.right_speed = speed;
	          	curr_state->state = TURNING;
	    	}
        	break;
      	}
      	case DRIVING: {
          float diff_left = curr_state->substate.target_forward_distance - curr_state->substate.total_distance_traveled_left;
          float diff_right = curr_state->substate.target_forward_distance - curr_state->substate.total_distance_traveled_right;
          float avg  = (diff_left + diff_right) / 2;

	        if (input_state.button_pressed || !input_state.has_recently_connected) {
	          	curr_state->state = OFF;
	        } else if (curr_state->substate.substate == FORWARD && avg < distance_threshold) {
	            curr_state->curr_orientation_angle = curr_state->curr_orientation_angle + curr_state->substate.relative_orientation_angle;
				curr_state->turn_angle = -1 * curr_state->curr_orientation_angle;
	            curr_state->state = END_TURNING;
	        } else {
			  outputs_t output_substate = substate_transition(input_state, curr_state);
	          output.left_speed = output_substate.left_speed;
	          output.right_speed = output_substate.right_speed;
	          curr_state->state = DRIVING;
	        }
        	break;
     	}
		case END_TURNING: {

			curr_state->curr_orientation_angle = input_state.gyro_integration_z_value; 
	        float diff = angle_modulo(curr_state->turn_angle - curr_state->curr_orientation_angle);

			if (input_state.button_pressed || !input_state.has_recently_connected) {
	          	curr_state->state = OFF;
			} else if (fabs(diff) < angle_threshold) {
	        	output.notify_ack = true;
	            curr_state->acknowledged_val = 0;
	            curr_state->state = WAITING;
	        } else {
				int8_t sign = (2 * (diff > 0)) - 1;
	          	int16_t speed = sign * fmax(.8 * fabs(diff), min_angle_speed);
				output.left_speed = -speed;
				output.right_speed = speed;
	          	curr_state->state = END_TURNING;
			}
			break;
		}
     	default: {
     		break;
     	}
	}
	output.notify_val  = curr_state->acknowledged_val;
	// if there was a transition, run transition in and out functions
	if (curr_state->state != old_state) {
     		transition_out(input_state, curr_state, old_state);
     		transition_in(input_state, curr_state);
    }
	print_state(*curr_state, output.display_line_0, output.display_line_1);
	return output;
}


void print_substate(system_state_t current_state, char* display_line_0, char* display_line_1){
	switch(current_state.substate.substate){
		case FORWARD: 
			snprintf(display_line_0, 16, "TARGET: %.2f", current_state.distance_to_travel);
			snprintf(display_line_1, 16, "X: %.2f Y: %.2f", current_state.position_x, current_state.position_y);
			break;
		case BACKWARD: {
			snprintf(display_line_0, 16, "REVERSE");
			snprintf(display_line_1, 16, "X: %.2f Y: %.2f", current_state.position_x, current_state.position_y);
			break;
		}
		case STOPPED: {
			snprintf(display_line_0, 16, "OUCH");
			if(current_state.substate.most_recent_bump == LEFT_BUMP){
				snprintf(display_line_1, 16, "Left/Center Hit");

			}
			else if(current_state.substate.most_recent_bump == RIGHT_BUMP){
				snprintf(display_line_1, 16, "Right Hit");
			}
			else {
				snprintf(display_line_1, 16, "No Hit???");
			}
			break;
		}
		case ROTATING: {
			snprintf(display_line_0, 16, "TURN TARGET: %f", current_state.substate.turn_angle_substate);
			snprintf(display_line_1, 16, "ANGLE CUR: %f", current_state.curr_orientation_angle);
			break;
		}
		case AVOIDANCE: {
			snprintf(display_line_0, 16, "A: %.2f,X: %.2f", current_state.curr_orientation_angle, current_state.position_x);
			snprintf(display_line_1, 16, "Y: %.2f", current_state.position_y);
			break;
		}
	}

}


void print_state(system_state_t current_state, char* display_line_0, char* display_line_1){
	switch(current_state.state){
		case OFF: {
			snprintf(display_line_0, 16, "OFF");
	        snprintf(display_line_1, 16, "");
			break;
	      }
		case WAITING: {
			snprintf(display_line_0, 16, "WAITING");
	        snprintf(display_line_1, 16, "");
			break;
	    }
		case TURNING: {
	        snprintf(display_line_0, 16, "TURN TARGET: %f", current_state.turn_angle);
	        snprintf(display_line_1, 16, "ANGLE CUR: %f", current_state.curr_orientation_angle);
			break;
	    }
		case DRIVING: {
			print_substate(current_state, display_line_0, display_line_1);
			break;
	    }
	    case END_TURNING: {
	        snprintf(display_line_0, 16, "TURN TARGET: %f", current_state.turn_angle);
	        snprintf(display_line_1, 16, "ANGLE CUR: %f", current_state.curr_orientation_angle);
			break;
	    }
	}
}

driving_substate_t init_substate() {
	driving_substate_t ret_substate = {0};
	ret_substate.substate = FORWARD;
	return ret_substate;
}

void substate_transition_in(inputs_t input_state, driving_substate_t* curr_state){

	switch(curr_state->substate) {
		case FORWARD: {
			curr_state->previous_left_encoder = input_state.left_encoder;
			curr_state->previous_right_encoder = input_state.right_encoder;
			curr_state->total_distance_traveled_left = 0;
			curr_state->total_distance_traveled_right = 0;
			break;
		}
		case BACKWARD: {
			curr_state->previous_left_encoder = input_state.left_encoder;
			curr_state->previous_right_encoder = input_state.right_encoder;
			curr_state->total_distance_traveled_left = 0;
			curr_state->total_distance_traveled_right = 0;
			break;
		}
		case STOPPED: {
			curr_state->stopping_timer = 0;
			break;
		}
		case ROTATING: {
			stop_gyro_integration();
			start_gyro_integration();
			curr_state->relative_orientation_angle = 0;
			break;
		}
		case AVOIDANCE: {

			curr_state->previous_left_encoder = input_state.left_encoder;
			curr_state->previous_right_encoder = input_state.right_encoder;
			curr_state->total_distance_traveled_left = 0;
			curr_state->total_distance_traveled_right = 0;
			break;
		}
	}

}

void substate_transition_out(inputs_t input_state, driving_substate_t* curr_state, substates old_state){

	switch(old_state) {
		case FORWARD: {
			curr_state->total_distance_traveled_left = 0;
			curr_state->total_distance_traveled_right = 0;
			break;
		}
		case BACKWARD: {
			curr_state->total_distance_traveled_left = 0;
			curr_state->total_distance_traveled_right = 0;
			break;
		}
		case STOPPED: {
			curr_state->stopping_timer = 0;
			break;
		}
		case ROTATING: {
			stop_gyro_integration();
			curr_state->turn_angle_substate = 0;
			curr_state->relative_orientation_angle = 0;
			break;
		}
		case AVOIDANCE: {
			curr_state->total_distance_traveled_left = 0;
			curr_state->total_distance_traveled_right = 0;
			break;		}
	}

}

void turning_controls(float target_angle, inputs_t input_state, system_state_t* curr_state, outputs_t* output) {
	// Compute motor inputs for robot to turn a target angle (degrees)

	// Update current orientation
	curr_state->substate.relative_orientation_angle = input_state.gyro_integration_z_value; 

	// Compute angle error
	float diff = angle_modulo(target_angle - curr_state->substate.relative_orientation_angle);

	// Calculate sign and magntidue of motor input
	int8_t sign = (2 * (diff > 0)) - 1;
	float magnitude = sign * fmax(.8 * fabs(diff), min_angle_speed);

	// Set motor inputs for both wheels
	int16_t speed = sign * fmax(.8 * fabs(diff), min_angle_speed);
	output->left_speed = -speed;
	output->right_speed = speed;
	curr_state->substate.substate = ROTATING;
}

void driving_controls(float target_distance, inputs_t input_state, system_state_t* curr_state, outputs_t* output) {
	// Compute motor inputs for robot drive a target distance (meters)

	// Compute distance error on both wheels
	float diff_left = target_distance - curr_state->substate.total_distance_traveled_left;
	float diff_right = target_distance - curr_state->substate.total_distance_traveled_right;
	
	// Compute distance mismatch between wheels
	float wheel_diff = diff_right - diff_left;

	// Update distance traveled by both wheels
	float dist_traveled_left = measure_distance(input_state.left_encoder, curr_state->substate.previous_left_encoder);
	float dist_traveled_right = measure_distance(input_state.right_encoder, curr_state->substate.previous_right_encoder);
	curr_state->substate.total_distance_traveled_left += dist_traveled_left;
	curr_state->substate.total_distance_traveled_right += dist_traveled_right;

	// Update encoder values for next distance calculation
	curr_state->substate.previous_left_encoder = input_state.left_encoder;
	curr_state->substate.previous_right_encoder = input_state.right_encoder;

	// Update distance traveled by car
	float avg_dist = (dist_traveled_right + dist_traveled_left) / 2;
	curr_state->position_x += cosf(curr_state->curr_orientation_angle * M_PI / 180) * avg_dist;
	curr_state->position_y += sinf(curr_state->curr_orientation_angle * M_PI / 180) * avg_dist;

	// Calculate appropriate sign of motor input for both wheels
	int8_t sign_left = (2 * (diff_left > 0)) - 1;
	int8_t sign_right = (2 * (diff_right > 0)) - 1;

	// Calculate magnitude of motor input for both wheels
	float left_magnitude = fmax(fabs(k_dist * diff_left - k_diff * wheel_diff), min_drive_speed);
	float right_magnitude = fmax(fabs(k_dist * diff_right + k_diff * wheel_diff), min_drive_speed);

	// Set motor inputs for both wheels
	output->left_speed = sign_left * left_magnitude;
	output->right_speed = sign_right * right_magnitude;
}

outputs_t substate_transition(inputs_t input_state, system_state_t* curr_state){

	outputs_t output = {0};
	substates old_state = curr_state->substate.substate;

	switch (curr_state->substate.substate) {
	case FORWARD: {
		// Controls for driving forward

		if(input_state.bump_left || input_state.bump_right){
			// Obstacle detection

			curr_state->substate.most_recent_bump = input_state.bump_left ? LEFT_BUMP : RIGHT_BUMP;
			curr_state->substate.avoidance_distance += AVOID_DIST_INCR;
			curr_state->substate.substate = STOPPED;
			break;
		} else {
			// Have the robot drive forward the correct distance to the target

			driving_controls(curr_state->substate.target_forward_distance, input_state, curr_state, &output);
			curr_state->substate.substate = FORWARD;
			break;
		}
	}
	case STOPPED: {
		if(curr_state->substate.stopping_timer > TIME_MAX){
			curr_state->substate.substate = BACKWARD;
			break;
		}
		else {
			curr_state->substate.stopping_timer += 1;
			output.left_speed = 0;
			output.right_speed = 0;
			break;
		}
	}
	case BACKWARD: {
		// Controls for driving backwards

		// Compute our distance error
		float diff_left = curr_state->substate.avoidance_distance - curr_state->substate.total_distance_traveled_left;
		float diff_right = curr_state->substate.avoidance_distance - curr_state->substate.total_distance_traveled_right;
		float avg_diff = (diff_left + diff_right) / 2;
		if(fabs(avg_diff) < distance_threshold){
			// After reversing far enough, determine which direction to turn given our most recent bump

			if(curr_state->substate.most_recent_bump == LEFT_BUMP){
				curr_state->substate.turn_angle_substate = -45;
			}
			else if(curr_state->substate.most_recent_bump == RIGHT_BUMP){
				curr_state->substate.turn_angle_substate = 45;
			}
			else {
				// should never happen; center bump is handeled in RIGHT_BUMP case
				curr_state->turn_angle = curr_state->curr_orientation_angle;
			}
			curr_state->substate.next_state_turning = AVOIDANCE;
			curr_state->substate.substate = ROTATING;
			break;
		}
		else {
			// Have the robot drive backward away from an obstacle

			driving_controls(BACKWARD_DIST, input_state, curr_state, &output);
			curr_state->substate.substate = BACKWARD;
			break;
		}
	}
	case ROTATING: {
		// Controls for turning toward waypoint

		// Compute our angle error
	    float diff = angle_modulo(curr_state->substate.turn_angle_substate - curr_state->substate.relative_orientation_angle);
		if (fabs(diff) < angle_threshold) {
			curr_state->curr_orientation_angle = angle_modulo(curr_state->curr_orientation_angle + curr_state->substate.relative_orientation_angle);
			curr_state->substate.substate = curr_state->substate.next_state_turning;
			break;
		}
		else {
			// Have the robot orient toward the waypoint

			turning_controls(curr_state->substate.turn_angle_substate, input_state, curr_state, &output);
			curr_state->substate.substate = ROTATING;
			break;
		}
	}
	case AVOIDANCE: {
		// Controls for driving forward away from the obstacle
				
		// Compute our distance error
		float diff_left = curr_state->substate.avoidance_distance - curr_state->substate.total_distance_traveled_left;
		float diff_right = curr_state->substate.avoidance_distance - curr_state->substate.total_distance_traveled_right;
		float avg_diff = (diff_left + diff_right) / 2;
		if(input_state.bump_left || input_state.bump_right){
			// Obstacle detection

			curr_state->substate.most_recent_bump = input_state.bump_left ? LEFT_BUMP : RIGHT_BUMP;
			curr_state->substate.substate = STOPPED;
			break;
		} else if(fabs(avg_diff) < distance_threshold){
			// Re-compute target angles and distances now that we are around the obstacle
			
			// Compute error to target
			float xdiff = curr_state->distance_to_travel - curr_state->position_x;
			float ydiff = -curr_state->position_y;

			// Distance and angle to target
			curr_state->substate.turn_angle_substate =  angle_modulo(atan2f(ydiff, xdiff) * 180 / M_PI - curr_state->curr_orientation_angle);
			curr_state->substate.target_forward_distance = sqrtf(xdiff * xdiff + ydiff * ydiff);

			curr_state->substate.next_state_turning = FORWARD;
			curr_state->substate.substate = ROTATING;
		} else {
			// Have the Romi drive around the obstacle

			driving_controls(curr_state->substate.avoidance_distance, input_state, curr_state, &output);
			curr_state->substate.substate = AVOIDANCE;
			break;
		}
	}
	default:
		break;
	}
	// if there was a substate transition, run the transition in and transition out functions
	if (curr_state->substate.substate != old_state) {
     		substate_transition_out(input_state, &(curr_state->substate), old_state);
     		substate_transition_in(input_state, &(curr_state->substate));
    }
	return output;
}
