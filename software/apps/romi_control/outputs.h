#ifndef OUTPUTS_H_
#define OUTPUTS_H_

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>


#include "app_error.h"
#include "app_timer.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_spi.h"

#include "buckler.h"
#include "display.h"
#include "kobukiActuator.h"
#include "kobukiSensorPoll.h"
#include "kobukiSensorTypes.h"
#include "kobukiUtilities.h"
#include "lsm9ds1.h"
#include "simple_ble.h"

#include "helpers.h"

//defines output data, such as BLE ack, motor values, and LED Display strings
typedef struct outputs {
	bool notify_ack;
	int notify_val;
	int16_t left_speed;
	int16_t right_speed;
	char display_line_0[16];
	char display_line_1[16];
} outputs_t;


// Takes in all output data from the FSM and sends it to sensors to get done. 
void do_outputs(outputs_t* output);

#endif /* OUTPUTS_H_ */