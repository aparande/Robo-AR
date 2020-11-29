#include "inputs.h"


inputs_t get_inputs() {
	inputs_t return_inputs = {0};

	
	return_inputs.waypoint_distance = waypoints[0];
	return_inputs.waypoint_angle = waypoints[1];
	int status = kobukiSensorPoll(&sensors);
	printf("%d\n", status);

	return_inputs.right_encoder = sensors.rightWheelEncoder;
	return_inputs.left_encoder = sensors.leftWheelEncoder;

	return_inputs.button_pressed = is_button_pressed(&sensors);
	return_inputs.bump_right = sensors.bumps_wheelDrops.bumpRight || sensors.bumps_wheelDrops.bumpCenter;
	return_inputs.bump_left = sensors.bumps_wheelDrops.bumpLeft;

	return_inputs.is_integrating = nrfx_timer_is_enabled(&gyro_timer);

	if(return_inputs.is_integrating) {
		return_inputs.gyro_integration_z_value = lsm9ds1_read_gyro_integration().z_axis;
	}
	return_inputs.has_recently_connected = connected;


}

void stop_gyro_integration() {
	lsm9ds1_stop_gyro_integration();
}

void start_gyro_integration() {
	error_code = lsm9ds1_start_gyro_integration();
	APP_ERROR_CHECK(error_code);
}

