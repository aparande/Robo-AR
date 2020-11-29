#include "states.h"


system_state_t init_state() {
	system_state_t ret_system = {0};
	ret_system.state = OFF;
	return ret_system;

}



output_t transition(system_state_t* curr_state) {








}

void print_state(system_state_t current_state, char* display_line_1, char* display_line_2){
	switch(current_state.state){
	case OFF: {
		snprintf(display_line_1, 16, "OFF");
        snprintf(display_line_2, 16, "");
		break;
      }
	case WAITING: {
		snprintf(display_line_1, 16, "WAITING");
        snprintf(display_line_2, 16, "");
		break;
    }
	case TURNING: {
        snprintf(display_line_1, 16, "TURN TARGET: %f", current_state.curr_waypoint_angle);
        snprintf(display_line_2, 16, "ANGLE CUR: %f", current_state.curr_orientation_angle);
		break;
    }
	case DRIVING_FORWARD: {
        snprintf(display_line_1, 16, "TARGET: %.2f", current_state.curr_waypoint_distance);
        snprintf(display_line_2, 16, "X: %.2f Y: %.2f", current_state.position_x, current_state.position_y);
		break;
    }
    case DRIVING_BACKWARD: {
    	snprintf(display_line_1, 16, "REVERSE");
    	snprintf(display_line_2, 16, "X: %.2f Y: %.2f", current_state.position_x, current_state.position_y);

    }
    case DRIVING_STOP_HIT: {
    	snprintf(display_line_1, 16, "OUCH");
    	if(current_state.most_recent_bump == LEFT_BUMP){
    		snprintf(display_line_2, 16, "Left/Center Hit");

    	}
    	else if(current_state.most_recent_bump == RIGHT_BUMP){
    		snprintf(display_line_2, 16, "Right Hit");
    	}
    	else {
    		snprintf(display_line_2, 16, "No Hit???");
    	}
    }
    case DRIVING_TURNING: {
    	snprintf(display_line_1, 16, "TURN TARGET: %f", current_state.turn_angle);
        snprintf(display_line_2, 16, "ANGLE CUR: %f", current_state.curr_orientation_angle);
		break;
    }
    case DRIVING_AVOIDANCE: {
    	snprintf(display_line_1, 16, "TARGET: %.2f", current_state.avoidance_distance);
        snprintf(display_line_2, 16, "X: %.2f Y: %.2f", current_state.position_x, current_state.position_y);
		break;
    }
    
}