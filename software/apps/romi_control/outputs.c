#include "outputs.h"

void do_outputs(outputs_t output) {
	kobukiDriveDirect(output.left_speed, output.right_speed);
	acknowledged = output.notify_ack;
	if (output.notify_ack >= 0) {
		simple_ble_notify_char(&ack_char);
	}
}