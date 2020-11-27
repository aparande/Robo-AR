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

#include "states.h"


// I2C manager
NRF_TWI_MNGR_DEF(twi_mngr_instance, 5, 0);

// global variables
KobukiSensors_t sensors = {0};
const float CLOCK_FREQ = 32768.0;

// Intervals for advertising and connections
static simple_ble_config_t ble_config = {
        // c0:98:e5:49:xx:xx
        .platform_id       = 0x49,    // used as 4th octect in device BLE address
        .device_id         = 0xEEC5, 
        .adv_name          = "DNEG", // used in advertisements if there is room
        .adv_interval      = MSEC_TO_UNITS(1000, UNIT_0_625_MS),
        .min_conn_interval = MSEC_TO_UNITS(100, UNIT_1_25_MS),
        .max_conn_interval = MSEC_TO_UNITS(200, UNIT_1_25_MS),
};

//4607eda0-f65e-4d59-a9ff-84420d87a4ca
static simple_ble_service_t drive_command_service = {{
    .uuid128 = {0xca,0xa4,0x87,0x0d,0x42,0x84,0xff,0xA9,
                0x59,0x4D,0x5e,0xf6,0xa0,0xed,0x07,0x46}
}};

//Declare characteristics and variables for your service
static simple_ble_char_t drive_command_char = {.uuid16 = 0xeda1};
static simple_ble_char_t drive_distance_char = {.uuid16 = 0xeda2};
static simple_ble_char_t ack_char = {.uuid16 = 0xeda3};
simple_ble_app_t* simple_ble_app;

//drivecommand for romi: left motor input, right motor input, time
//should really be using a struct for this... motor inputs have to be int16_t. Time is float.
//Right now I just case motor inputs to int16_t before they get used.
float drive_command[3] = {0, 0, 0};
//distance driven by both wheels: left distance, right distance, time
float drive_distance[3] = {0, 0, 0};
uint8_t acknowledged = 0;
states state = OFF;
uint32_t start_ticks = 0;

void readInput() {
    printf("Bluetooth message recieved\n");
    if (state == WAITING) {
        printf("Left Input: %f\n", drive_command[0]);
        printf("Right Input: %f\n", drive_command[1]);
        printf("Time: %f\n", drive_command[2]);
        acknowledged = 1;
    }
}

void ble_evt_write(ble_evt_t const* p_ble_evt) {
    //logic for each characteristic and related state changes
    //Try not to modify the state here...
    readInput();
}


static float measure_distance(uint16_t current_encoder, uint16_t previous_encoder) {
  const float CONVERSION = 0.0006108;
  float distance = 0;
  if (current_encoder < previous_encoder) {
      if (previous_encoder - current_encoder > 30000) {
        distance = (current_encoder - previous_encoder + 655365);
      }
      else {
        distance = current_encoder - previous_encoder;
      }
  }
  else {
      if (current_encoder  - previous_encoder > 30000) {
        distance = (current_encoder - previous_encoder - 655365);
      }
      else {
        distance = current_encoder - previous_encoder;
      }
  }
  float val = CONVERSION * distance;
  if (fabs(val) > 300) {
     val = 0;
  }
  return val;
}

static float measure_time(void) {
    uint32_t cur_ticks = app_timer_cnt_get();
    uint32_t tick_diff = app_timer_cnt_diff_compute(cur_ticks, start_ticks);
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
        snprintf(buf, 16, "Time Left: %.2f", drive_command[2] - measure_time());
		display_write(buf, DISPLAY_LINE_0);
        snprintf(buf, 16, "L: %.2f R: %.2f", drive_distance[0], drive_distance[1]);
		display_write(buf, DISPLAY_LINE_1);
		break;
    }
   }
}

static void single_shot_timer_handler(void * p_context)
{
    printf("Dummy fired!\n");
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
      sizeof(drive_distance), (uint8_t*)&drive_distance,
      &drive_command_service, &drive_distance_char);

  simple_ble_add_characteristic(1, 0, 1, 0,
      sizeof(acknowledged), (uint8_t*)&acknowledged,
      &drive_command_service, &ack_char);

  // Start Advertising
  simple_ble_adv_only_name();

  // initialize LEDs
  nrf_gpio_pin_dir_set(23, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(24, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(25, NRF_GPIO_PIN_DIR_OUTPUT);

  nrf_gpio_pin_set(23);
  nrf_gpio_pin_set(24);
  nrf_gpio_pin_set(25);



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

  nrf_drv_clock_init();
  //APP_ERROR_CHECK(err_code);
  nrf_drv_clock_lfclk_request(NULL);  
  error_code = app_timer_init();
  APP_ERROR_CHECK(error_code);

  //Need to make a timer for a dumb reason
  APP_TIMER_DEF(m_single_shot_timer_id);
  error_code = app_timer_create(&m_single_shot_timer_id,
                   APP_TIMER_MODE_SINGLE_SHOT,
                   single_shot_timer_handler);
  APP_ERROR_CHECK(error_code);
  error_code = app_timer_start(m_single_shot_timer_id, APP_TIMER_TICKS(100000), NULL);
  APP_ERROR_CHECK(error_code);
  printf("Timer initialized!\n");

  float encoder_prev_left = 0;
  float encoder_prev_right = 0;
  float encoder_cur_left = 0;
  float encoder_cur_right = 0;

  // loop forever, running state machine
  while (1) {
    // read sensors from robot
    int status = kobukiSensorPoll(&sensors);
    // print out current state
    print_state(state);
    // switch into appropriate states
    // OFF - No response to any signals until button pressed to go to WAITING
    // WAITING - Do nothing until waypoint is recieved. Then go to TURNING
    // Driving - Drive the amount specified by waypoint. Then go back to WAITING
    switch(state) {
      case OFF: {
        if (is_button_pressed(&sensors)) {
          nrf_gpio_pin_clear(23);
          state = WAITING;
        } else {
          state = OFF;
          kobukiDriveDirect(0, 0);
        }
        break;
      }
      case WAITING: {
        if (is_button_pressed(&sensors)) {
          nrf_gpio_pin_set(23);
          state = OFF;
        } else if (acknowledged==1) {
          nrf_gpio_pin_clear(24);
          encoder_prev_left = sensors.leftWheelEncoder;
          encoder_prev_right = sensors.rightWheelEncoder;
          start_ticks = app_timer_cnt_get();
          state = DRIVING;
        } else {
          state = WAITING;
          kobukiDriveDirect(0, 0);
        }
        break;
      }
      case DRIVING: {
        float cur_time = measure_time();
        if (is_button_pressed(&sensors)) {
            nrf_gpio_pin_set(23);
            nrf_gpio_pin_set(24);
            drive_distance[0] = 0;
            drive_distance[1] = 0;
            drive_distance[2] = 0;
            state = OFF;
        } else if (cur_time > drive_command[2]) {
            nrf_gpio_pin_set(24);
            drive_distance[0] = 0;
            drive_distance[1] = 0;
            drive_distance[2] = 0;
            acknowledged=0;
            state = WAITING;
        } else {
          state = DRIVING;
          encoder_cur_left = sensors.leftWheelEncoder;
          encoder_cur_right = sensors.rightWheelEncoder;
          drive_distance[0] += measure_distance(encoder_cur_left, encoder_prev_left);
          drive_distance[1] += measure_distance(encoder_cur_right, encoder_prev_right);
          drive_distance[2] = cur_time;
          encoder_prev_left = encoder_cur_left;
          encoder_prev_right = encoder_cur_right;
          kobukiDriveDirect((int16_t) drive_command[0], (int16_t) drive_command[1]);
        }
        break;
      }
    }
  }
}

