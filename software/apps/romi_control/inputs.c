#include "inputs.h"


inputs_t get_inputs() {
	inputs_t return_inputs = {0};

	int status = kobukiSensorPoll(&sensors);

	return_inputs.right_encoder = sensors.rightWheelEncoder;
	return_inputs.left_encoder = sensors.leftWheelEncoder;

	return_inputs.button_pressed = is_button_pressed(&sensors);
	return_inputs.bump_right = sensors.bumps_wheelDrops.bumpRight || sensors.bumps_wheelDrops.bumpCenter;
	return_inputs.bump_left = sensors.bumps_wheelDrops.bumpLeft;


	return_inputs.is_integrating = is_gyro_integrating;
	if(return_inputs.is_integrating) {
		printf("integrating!\n");
		return_inputs.gyro_integration_z_value = angle_modulo(lsm9ds1_read_gyro_integration().z_axis);
	}
	return_inputs.has_recently_connected = connected;
	return_inputs.new_waypoint_written = new_waypoint_written;
	if (new_waypoint_written) {
		return_inputs.waypoint_distance = waypoint[0];
		return_inputs.waypoint_angle = angle_modulo(waypoint[1]);
		printf("Waypoint Distance: %f\n", return_inputs.waypoint_distance);
		printf("Waypoint Angle: %f\n", return_inputs.waypoint_angle);
	}
	new_waypoint_written = false;
	return return_inputs;
}

void stop_gyro_integration() {
	lsm9ds1_stop_gyro_integration();
	is_gyro_integrating = false;
}

void start_gyro_integration() {
	error_code = lsm9ds1_start_gyro_integration();
	APP_ERROR_CHECK(error_code);
	is_gyro_integrating = true;
}

