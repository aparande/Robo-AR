// Robot Template app
// // Framework for creating applications that control the Kobuki robot

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "app_error.h"
#include "app_timer.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_drv_clock.h"
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

#include "states.h"

#define CONVERSION 0.0006108
#define CLOCK_FREQ 32768.0

// I2C manager
NRF_TWI_MNGR_DEF(twi_mngr_instance, 5, 0);

// Global variables
KobukiSensors_t sensors = {0};

// Intervals for advertising and connections
static simple_ble_config_t ble_config = {
        // MAC address of form c0:98:e5:49:xx:xx, XX:XX is platform_id
        .platform_id       = 0x51,    // used as 4th octect in device BLE address
        .device_id         = 0xEEC5, 
        .adv_name          = "Robo-AR", // used in advertisements if there is room
        .adv_interval      = MSEC_TO_UNITS(1000, UNIT_0_625_MS),
        .min_conn_interval = MSEC_TO_UNITS(100, UNIT_1_25_MS),
        .max_conn_interval = MSEC_TO_UNITS(200, UNIT_1_25_MS),
};

// Initialize BLE service
static simple_ble_service_t drive_command_service = {{
    .uuid128 = {0xca,0xa4,0x87,0x0d,0x42,0x84,0xff,0xA9,
                0x59,0x4D,0x5e,0xf6,0xa0,0xed,0x07,0x46}
}};

//Declare characteristics and variables for your service
static simple_ble_char_t drive_command_char = {.uuid16 = 0xeda1};
static simple_ble_char_t drive_data_char = {.uuid16 = 0xeda2};
static simple_ble_char_t ack_char = {.uuid16 = 0xeda3};
static simple_ble_char_t data_ready_char = {.uuid16 = 0xeda4};
simple_ble_app_t* simple_ble_app;

// Drive command for romi: left motor input, right motor input, time
float drive_command[3] = {0, 0, 0};

// Distance driven by both wheels: left distance, right distance, time

// Number of data points
int n_points = 10;

// Array to send over BLE to computer
float drive_data[] = {0,0,0,0,0,0,0,0,0,0,                        
                        0,0,0,0,0,0,0,0,0,0,
                        0,0,0,0,0,0,0,0,0,0};

// Array to hold data points as they are collected 
float cur_data[] = {0,0,0,0,0,0,0,0,0,0,                        
                        0,0,0,0,0,0,0,0,0,0,
                        0,0,0,0,0,0,0,0,0,0};

//Index of data point to write to
int point_idx = 0;
bool data_ready = false;

static void clear_data(bool keep) {
    // Clear data for more writes

    // If keep, first point will continue from old data. Else, overwrite first point
    if (keep) {
       cur_data[0] = cur_data[n_points * 3 - 3];
       cur_data[1] = cur_data[n_points * 3 - 2];
    } else {
        cur_data[0] = 0;
        cur_data[1] = 0;
        cur_data[2] = 0;
    }
}

static void copy_data() {
    // Copy current data over so it can be logged

    memcpy(drive_data, cur_data, point_idx * 3 * sizeof(float));
}

// Initialize state variables

uint8_t acknowledged = 0;
states state = OFF;
uint32_t start_ticks = 0;
bool connected = false;

void ble_evt_connected(ble_evt_t const* p_ble_evt) {
    connected = true;
}
void ble_evt_write(ble_evt_t const* p_ble_evt) {
    if (simple_ble_is_char_event(p_ble_evt, &data_ready_char)) {
        data_ready = false;
    } else if (simple_ble_is_char_event(p_ble_evt, &drive_command_char)) {
      if (state == WAITING) {
          acknowledged = 1;
      }
    }
}

static float measure_distance(uint16_t current_encoder, uint16_t previous_encoder) {
  float distance = CONVERSION * (current_encoder - previous_encoder);

  return distance;
}

static float measure_time(void) {
    // Get time from the start of timer

    uint32_t cur_ticks = app_timer_cnt_get();
    uint32_t tick_diff = app_timer_cnt_diff_compute(cur_ticks, start_ticks);

    // Need to divide ticks by clock frequency
    float time = tick_diff/CLOCK_FREQ;
    return time;
}

void print_state(states current_state){
    char buf[16];
	switch(current_state){
	case OFF: {
		display_write("OFF", DISPLAY_LINE_0);
        snprintf(buf, 16, "");
		display_write(buf, DISPLAY_LINE_1);
		break;
      }
	case WAITING: {
		display_write("WAITING", DISPLAY_LINE_0);
        snprintf(buf, 16, "");
		display_write(buf, DISPLAY_LINE_1);
		break;
    }
	case DRIVING: {
        if (point_idx > 0) { 
        snprintf(buf, 16, "Time Left: %.2f", drive_command[2] - cur_data[point_idx*3 - 1]);
		display_write(buf, DISPLAY_LINE_0);
        snprintf(buf, 16, "L: %.2f R: %.2f", cur_data[point_idx*3 - 3], cur_data[point_idx*3 - 2]);
		display_write(buf, DISPLAY_LINE_1);
        }
        break;
    }
   }
}

// Timing doesn't work without this...
static void dummy_handler(void * p_context)
{
  // We don't need to do anything here :)
}

int main(void) {
  ret_code_t error_code = NRF_SUCCESS;

  // initialize RTT library
  error_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(error_code);
  NRF_LOG_DEFAULT_BACKENDS_INIT();
  printf("Log initialized!\n");

  // Setup BLE
  simple_ble_app = simple_ble_init(&ble_config);

  simple_ble_add_service(&drive_command_service);

  //Register your characteristics
  simple_ble_add_characteristic(1, 1, 0, 0,
      sizeof(drive_command), (uint8_t*)&drive_command,
      &drive_command_service, &drive_command_char);

  simple_ble_add_characteristic(1, 0, 1, 0,
      sizeof(drive_data), (uint8_t*)&drive_data,
      &drive_command_service, &drive_data_char);

  simple_ble_add_characteristic(1, 0, 1, 0,
      sizeof(acknowledged), (uint8_t*)&acknowledged,
      &drive_command_service, &ack_char);

  simple_ble_add_characteristic(1, 1, 1, 0,
      sizeof(data_ready), (uint8_t*)&data_ready,
      &drive_command_service, &data_ready_char);

  // Start Advertising
  simple_ble_adv_only_name();

  // initialize display
  nrf_drv_spi_t spi_instance = NRF_DRV_SPI_INSTANCE(1);
  nrf_drv_spi_config_t spi_config = {
    .sck_pin = BUCKLER_LCD_SCLK,
    .mosi_pin = BUCKLER_LCD_MOSI,
    .miso_pin = BUCKLER_LCD_MISO,
    .ss_pin = BUCKLER_LCD_CS,
    .irq_priority = NRFX_SPI_DEFAULT_CONFIG_IRQ_PRIORITY,
    .orc = 0,
    .frequency = NRF_DRV_SPI_FREQ_4M,
    .mode = NRF_DRV_SPI_MODE_2,
    .bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST
  };
  error_code = nrf_drv_spi_init(&spi_instance, &spi_config, NULL, NULL);
  APP_ERROR_CHECK(error_code);
  display_init(&spi_instance);
  printf("Display initialized!\n");

  // initialize i2c master (two wire interface)
  nrf_drv_twi_config_t i2c_config = NRF_DRV_TWI_DEFAULT_CONFIG;
  i2c_config.scl = BUCKLER_SENSORS_SCL;
  i2c_config.sda = BUCKLER_SENSORS_SDA;
  i2c_config.frequency = NRF_TWIM_FREQ_100K;
  error_code = nrf_twi_mngr_init(&twi_mngr_instance, &i2c_config);
  APP_ERROR_CHECK(error_code);
  lsm9ds1_init(&twi_mngr_instance);
  printf("IMU initialized!\n");

  // initialize Kobuki
  kobukiInit();
  printf("Kobuki initialized!\n");

  // Initialize timer
  nrf_drv_clock_init();
  nrf_drv_clock_lfclk_request(NULL);  
  error_code = app_timer_init();
  APP_ERROR_CHECK(error_code);
  APP_TIMER_DEF(m_repeated_timer_id);
  error_code = app_timer_create(&m_repeated_timer_id,
                   APP_TIMER_MODE_REPEATED,
                   dummy_handler);
  APP_ERROR_CHECK(error_code);
  error_code = app_timer_start(m_repeated_timer_id, APP_TIMER_TICKS(100000), NULL);
  APP_ERROR_CHECK(error_code);
  printf("Timer initialized!\n");

  uint16_t encoder_prev_left = 0;
  uint16_t encoder_prev_right = 0;
  uint16_t encoder_cur_left = 0;
  uint16_t encoder_cur_right = 0;

  // loop forever, running state machine
  while (1) {
    // read sensors from robot
    int status = kobukiSensorPoll(&sensors);

    // print out current state
    print_state(state);

    // switch into appropriate states
    // OFF - No response to any signals until button pressed or connected. Then go to WAITING
    // WAITING - Do nothing until waypoint is recieved. Then go to TURNING
    // Driving - Drive the amount specified by waypoint. Then go back to WAITING
    switch(state) {
      case OFF: {
        // Do not respond to waypoint commands until button is pressed or device connected

        if (is_button_pressed(&sensors) || connected) {
          state = WAITING;
        } else {
          // Don't move in off state

          kobukiDriveDirect(0, 0);
          state = OFF;
        }
        break;
      }
      case WAITING: {
        // Idle until command recieved

        if (is_button_pressed(&sensors)) {
          state = OFF;
        } else if (acknowledged==1) {
          // When we recieved a command, set appropriate variables and start executing

          encoder_prev_left = sensors.leftWheelEncoder;
          encoder_prev_right = sensors.rightWheelEncoder;
          start_ticks = app_timer_cnt_get();
          state = DRIVING;
        } else {
          // Don't move in waiting state

          kobukiDriveDirect(0, 0);
          state = WAITING;
        }
        break;
      }
      case DRIVING: {
        // Drive until the timer expires

        float cur_time = measure_time();
        if (is_button_pressed(&sensors)) {
            clear_data(false);
            point_idx=0;
            state = OFF;
        } else if (cur_time > drive_command[2]) {
            acknowledged = 0;
            simple_ble_notify_char(&ack_char);
            copy_data();
            data_ready = true;
            clear_data(false);
            point_idx=0;
            state = WAITING;
        } else {
          state = DRIVING;
          encoder_cur_left = sensors.leftWheelEncoder;
          encoder_cur_right = sensors.rightWheelEncoder;
          if (point_idx > 0) {
              cur_data[point_idx*3] = cur_data[point_idx*3 - 3] + measure_distance(encoder_cur_left, encoder_prev_left);
              cur_data[point_idx*3 + 1] = cur_data[point_idx*3 - 2] + measure_distance(encoder_cur_right, encoder_prev_right);
          }
          cur_data[point_idx*3 + 2] = cur_time;
          encoder_prev_left = encoder_cur_left;
          encoder_prev_right = encoder_cur_right;
          point_idx++;
          if (point_idx == n_points) {
              copy_data();
              data_ready = true;
              clear_data(true);
              point_idx=0;
          }
          kobukiDriveDirect((int16_t) drive_command[0], (int16_t) drive_command[1]);
        }
        break;
      }
    }
  }
}

