#include "states.h"


system_state_t init_state() {
	system_state_t ret_system = {0};
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
		case DRIVING_FORWARD: {
			curr_state->total_distance_traveled_left = 0;
			curr_state->total_distance_traveled_right = 0;
			curr_state->previous_left_encoder = input_state.left_encoder;
			curr_state->previous_right_encoder = input_state.right_encoder;
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
			break;
		}
		case DRIVING_FORWARD: {
			curr_state->total_distance_traveled_left = 0;
			curr_state->total_distance_traveled_right = 0;
			curr_state->previous_left_encoder = 0;
			curr_state->previous_right_encoder = 0;
			curr_state->distance_to_travel = 0;
			break;
		}
	}
}

outputs_t transition(inputs_t input_state, system_state_t* curr_state) {
	outputs_t output = {0};
	output.notify_ack = -1;
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
        	break;
		}
		case WAITING: {
	        if (input_state.button_pressed || !input_state.has_recently_connected) {
	          	curr_state->state = OFF;
	        } else if (input_state.new_waypoint_written) {
	        	//transition out to turning
	        	output.notify_ack = 1;
	        	curr_state->turn_angle = input_state.waypoint_angle;
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
	        float diff = curr_state->turn_angle - curr_state->curr_orientation_angle;
	        if (input_state.button_pressed || !input_state.has_recently_connected) {
	          	curr_state->state = OFF;
	        } else if (fabs(diff) < angle_threshold) {
	        	//transition out of turning
	        	//transition into driving
	        	curr_state->position_x = 0;
	        	curr_state->position_y = 0;
	        	curr_state->distance_to_travel = input_state.waypoint_distance;
	            curr_state->state = DRIVING_FORWARD;
	        }
	        else {
	          	int8_t sign = (2 * (diff > 0)) - 1;
	          	int16_t speed = sign * fmax(.8 * fabs(diff), 50);
				output.left_speed = -speed;
				output.right_speed = speed;
	          	curr_state->state = TURNING;
	    	}
        	break;
      	}
      	case DRIVING_FORWARD: {
	        float diff_left = curr_state->distance_to_travel - curr_state->total_distance_traveled_left;
	        float diff_right = curr_state->distance_to_travel - curr_state->total_distance_traveled_right;
	        float wheel_diff = diff_right - diff_left;
	        curr_state->position_x = fmin(curr_state->total_distance_traveled_left, curr_state->total_distance_traveled_right);
	        int8_t sign_left = (2 * (diff_left > 0)) - 1;
	        int8_t sign_right = (2 * (diff_right > 0)) - 1;
	        if (input_state.button_pressed || !input_state.has_recently_connected) {
	          	curr_state->state = OFF;
	        } else if ((fmax(fabs(diff_left), fabs(diff_right)) < distance_threshold)) {
	            output.notify_ack = 0;
	            //transition out of driving
	            //transition into waiting
	            curr_state->state = WAITING;
	        } else {
	          curr_state->total_distance_traveled_left += measure_distance(input_state.left_encoder, curr_state->previous_left_encoder);
	          curr_state->total_distance_traveled_right += measure_distance(input_state.right_encoder, curr_state->previous_right_encoder);
		      curr_state->position_x = fmin(curr_state->total_distance_traveled_left, curr_state->total_distance_traveled_right);
	          //total_distance = (.5 * total_distance_left) + (.5 * total_distance_right);
	          curr_state->previous_left_encoder = input_state.left_encoder;
	          curr_state->previous_right_encoder = input_state.right_encoder;
	          //Wheel diff might not work if backwards :(
	          output.left_speed = sign_left * fmax(fabs(k_dist * diff_left - k_diff * wheel_diff), 70);
	          output.right_speed = sign_right * fmax(fabs(k_dist * diff_right + k_diff * wheel_diff), 70);
	          curr_state->state = DRIVING_FORWARD;
	        }
        	break;
     	}
     	default: {
     		break;
     	}
	}
	if (curr_state->state != old_state) {
     		transition_out(input_state, curr_state, old_state);
     		transition_in(input_state, curr_state);
    }
	print_state(*curr_state, output.display_line_0, output.display_line_1);
	return output;
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
		case DRIVING_FORWARD: {
	        snprintf(display_line_0, 16, "TARGET: %.2f", current_state.distance_to_travel);
	        snprintf(display_line_1, 16, "X: %.2f Y: %.2f", current_state.position_x, current_state.position_y);
			break;
	    }
	    case DRIVING_BACKWARD: {
	    	snprintf(display_line_0, 16, "REVERSE");
	    	snprintf(display_line_1, 16, "X: %.2f Y: %.2f", current_state.position_x, current_state.position_y);

	    }
	    case DRIVING_STOP_HIT: {
	    	snprintf(display_line_0, 16, "OUCH");
	    	if(current_state.most_recent_bump == LEFT_BUMP){
	    		snprintf(display_line_1, 16, "Left/Center Hit");

	    	}
	    	else if(current_state.most_recent_bump == RIGHT_BUMP){
	    		snprintf(display_line_1, 16, "Right Hit");
	    	}
	    	else {
	    		snprintf(display_line_1, 16, "No Hit???");
	    	}
	    }
	    case DRIVING_TURNING: {
	    	snprintf(display_line_0, 16, "TURN TARGET: %f", current_state.turn_angle);
	        snprintf(display_line_1, 16, "ANGLE CUR: %f", current_state.curr_orientation_angle);
			break;
	    }
	    case DRIVING_AVOIDANCE: {
	    	snprintf(display_line_0, 16, "TARGET: %.2f", current_state.avoidance_distance);
	        snprintf(display_line_1, 16, "X: %.2f Y: %.2f", current_state.position_x, current_state.position_y);
			break;
	    }
	}
}