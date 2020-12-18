#include "outputs.h"

// Takes outputs from FSM and sends the correct values to the sensors
void do_outputs(outputs_t output) {
	if (output.left_speed != 0 || output.right_speed !=0) {
	}
	kobukiDriveDirect(output.left_speed, output.right_speed);
	if (output.notify_ack) {
		acknowledged = output.notify_val;
		simple_ble_notify_char(&ack_char);
	}
}