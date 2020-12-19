#include "outputs.h"

// Takes outputs from FSM and sends the correct values to the motors and BLE ack
void do_outputs(outputs_t output) {
	kobukiDriveDirect(output.left_speed, output.right_speed);
	if (output.notify_ack) {
		acknowledged = output.notify_val;
		simple_ble_notify_char(&ack_char);
	}
}