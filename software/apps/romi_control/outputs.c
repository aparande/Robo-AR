#include "outputs.h"

void do_outputs(outputs_t* output) {
	if (output->left_speed != 0 || output->right_speed !=0) {
		printf("Left Speed: %i\n", output->left_speed);
		printf("Right Speed: %i\n", output->right_speed);
	}
	kobukiDriveDirect(output->left_speed, output->right_speed);
	if (output->notify_ack) {
		acknowledged = output->notify_val;
		simple_ble_notify_char(&ack_char);
	}
}