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
		case END_TURNING:
		case TURNING: {
			curr_state->curr_orientation_angle = 0;
			stop_gyro_integration();
			start_gyro_integration();
			break;
		}
		case DRIVING: {
			curr_state->substate = init_substate();
			curr_state->curr_orientation_angle = 0;
			curr_state->position_x = 0;
			curr_state->position_y = 0;
			curr_state->substate.previous_left_encoder = input_state.left_encoder;
			curr_state->substate.previous_right_encoder = input_state.right_encoder;
			break;
		}
	}
}

void transition_out(inputs_t input_state, system_state_t* curr_state, states old_state) {
	switch(old_state) {
		case END_TURNING:
		case TURNING: {
			stop_gyro_integration();
			curr_state->turn_angle = 0;
			curr_state->curr_orientation_angle = 0;
			break;
		}
		case DRIVING: {
			curr_state->substate = init_substate();
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
        	} else {
          		output.left_speed = 0;
          		output.right_speed = 0;
          		curr_state->state = OFF;
        	}
		}
		case WAITING: {
	        if (input_state.button_pressed || !input_state.has_recently_connected) {
	          	curr_state->state = OFF;
	        } else if (input_state.new_waypoint_written) {
	        	//transition out to turning
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
		case END_TURNING:
      	case TURNING: {
      		curr_state->curr_orientation_angle = input_state.gyro_integration_z_value; 
	        float diff = angle_modulo(curr_state->turn_angle - curr_state->curr_orientation_angle);
	        if (input_state.button_pressed || !input_state.has_recently_connected) {
	          	curr_state->state = OFF;
	        } else if (fabs(diff) < angle_threshold) {
				if (curr_state->state = END_TURNING) {
					//transition out of turning and into waiting
					output.notify_ack = true;
					curr_state->acknowledged_val = 0;
					curr_state->state = WAITING;
				} else {
					//transition out of turning and into driving
					curr_state->state = DRIVING;
				}
	        }
	        else {
	          	int8_t sign = (2 * (diff > 0)) - 1;
	          	int16_t speed = sign * fmax(.8 * fabs(diff), min_angle_speed);
				output.left_speed = -speed;
				output.right_speed = speed;
	    	}
        	break;
      	}
      	case DRIVING: {
	        float diff_x = curr_state->position_x - curr_state->distance_to_travel;
			float diff_y = curr_state->position_y;
			float dist = diff_x * diff_x + diff_y * diff_y;

	        if (input_state.button_pressed || !input_state.has_recently_connected) {
	          	curr_state->state = OFF;
	        } else if (dist < distance_threshold * distance_threshold) {
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
	}
	output.notify_val  = curr_state->acknowledged_val;
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
		case END_TURNING:
		case TURNING: {
	        snprintf(display_line_0, 16, "TURN TARGET: %f", current_state.turn_angle);
	        snprintf(display_line_1, 16, "ANGLE CUR: %f", current_state.curr_orientation_angle);
			break;
	    }
		case DRIVING: {
			print_substate(current_state, display_line_0, display_line_1);
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
		case AVOIDANCE:
		case BACKWARD:
		case FORWARD: {
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
	}

}

void substate_transition_out(inputs_t input_state, driving_substate_t* curr_state, substates old_state){
	switch(old_state) {
		case AVOIDANCE:
		case BACKWARD:
		case FORWARD: {
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
	}

}

outputs_t substate_transition(inputs_t input_state, system_state_t* curr_state){

	outputs_t output = {0};
	substates old_state = curr_state->substate.substate;

	switch (curr_state->substate.substate) {
	case FORWARD: {

		if(input_state.bump_left || input_state.bump_right){
			curr_state->substate.most_recent_bump = input_state.bump_left ? LEFT_BUMP : RIGHT_BUMP;
			curr_state->substate.avoidance_distance += AVOID_DIST_INCR;
			curr_state->substate.substate = STOPPED;
		} else {
			translation_control(curr_state, input_state, 1, &(output.left_speed), &(output.right_speed));
		}
		break;
	}
	case STOPPED: {
		if(curr_state->substate.stopping_timer > TIME_MAX){
			curr_state->substate.substate = BACKWARD;
		}
		else {
			curr_state->substate.stopping_timer += 1;
			output.left_speed = 0;
			output.right_speed = 0;
		}
		break;
	}
	case BACKWARD: {

		float avg_total_dist = (curr_state->substate.total_distance_traveled_left + curr_state->substate.total_distance_traveled_right) / 2;
		float diff = BACKWARD_DIST - avg_total_dist;

		if(fabs(diff) < distance_threshold){
			if(curr_state->substate.most_recent_bump == LEFT_BUMP){
				curr_state->substate.turn_angle_substate = -45;
			}
			else if(curr_state->substate.most_recent_bump == RIGHT_BUMP){
				curr_state->substate.turn_angle_substate = 45;
			}
			else {
				// should never happen
				curr_state->turn_angle = curr_state->curr_orientation_angle;
			}
			curr_state->substate.next_state_turning = AVOIDANCE;
			curr_state->substate.substate = ROTATING;
		}
		else {
			translation_control(curr_state, input_state, -1, &(output.left_speed), &(output.right_speed));
		}
		break;
	}
	case ROTATING: {
		curr_state->substate.relative_orientation_angle = input_state.gyro_integration_z_value; 
	    float diff = angle_modulo(curr_state->substate.turn_angle_substate - curr_state->substate.relative_orientation_angle);
	    
		if (fabs(diff) < angle_threshold) {
			curr_state->curr_orientation_angle = angle_modulo(curr_state->curr_orientation_angle + curr_state->substate.relative_orientation_angle);
			curr_state->substate.substate = curr_state->substate.next_state_turning;
		}
		else {
			int8_t sign = (2 * (diff > 0)) - 1;
			int16_t speed = sign * fmax(.8 * fabs(diff), min_angle_speed);
			output.left_speed = -speed;
			output.right_speed = speed;
			curr_state->substate.substate = ROTATING;
		}
		break;
	}
	case AVOIDANCE: {
				
		float diff_left = curr_state->substate.avoidance_distance - curr_state->substate.total_distance_traveled_left;
		float diff_right = curr_state->substate.avoidance_distance - curr_state->substate.total_distance_traveled_right;
		
		float avg_diff = (diff_left + diff_right) / 2;
		if(input_state.bump_left || input_state.bump_right){

			curr_state->substate.most_recent_bump = input_state.bump_left ? LEFT_BUMP : RIGHT_BUMP;
			curr_state->substate.substate = STOPPED;
		} else if(fabs(avg_diff) < distance_threshold){
			
			curr_state->substate.turn_angle_substate =  angle_modulo(atan2f(-curr_state->position_y, curr_state->distance_to_travel - curr_state->position_x) * 180 / M_PI - curr_state->curr_orientation_angle);
			curr_state->substate.next_state_turning = FORWARD;
			curr_state->substate.substate = ROTATING;
		} else {
			translation_control(curr_state, input_state, 1, &(output.left_speed), &(output.right_speed));
		}
		break;
	}
	}
	if (curr_state->substate.substate != old_state) {
     		substate_transition_out(input_state, &(curr_state->substate), old_state);
     		substate_transition_in(input_state, &(curr_state->substate));
    }
	return output;
}

void translation_control(system_state_t* curr_state, inputs_t input_state, int dir, int16_t* left_pwr, int16_t* right_pwr) {
  float diff_left = curr_state->distance_to_travel - curr_state->substate.total_distance_traveled_left;
  float diff_right = curr_state->distance_to_travel - curr_state->substate.total_distance_traveled_right;
  
  float wheel_diff = diff_right - diff_left;
  int8_t sign_left = (2 * (diff_left > 0)) - 1;
  int8_t sign_right = (2 * (diff_right > 0)) - 1;
  
  float dist_traveled_left = measure_distance(input_state.left_encoder, curr_state->substate.previous_left_encoder);
  float dist_traveled_right = measure_distance(input_state.right_encoder, curr_state->substate.previous_right_encoder);
  curr_state->substate.total_distance_traveled_left += dist_traveled_left;
  curr_state->substate.total_distance_traveled_right += dist_traveled_right;

  float avg_dist = dir * (dist_traveled_right + dist_traveled_left) / 2;
  curr_state->position_x += cosf(curr_state->curr_orientation_angle * M_PI / 180) * avg_dist;
  curr_state->position_y += sinf(curr_state->curr_orientation_angle * M_PI / 180) * avg_dist;

  curr_state->substate.previous_left_encoder = input_state.left_encoder;
  curr_state->substate.previous_right_encoder = input_state.right_encoder;
  *left_pwr = dir * sign_left * fmax(fabs(k_dist * diff_left - k_diff * wheel_diff), min_drive_speed);
  *right_pwr = dir * sign_right * fmax(fabs(k_dist * diff_right + k_diff * wheel_diff), min_drive_speed);

}
