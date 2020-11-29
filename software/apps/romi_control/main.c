// // Robot Template app
// // // Framework for creating applications that control the Kobuki robot

// #include <math.h>
// #include <stdbool.h>
// #include <stdint.h>
// #include <stdio.h>

// #include "app_error.h"
// #include "app_timer.h"
// #include "nrf.h"
// #include "nrf_delay.h"
// #include "nrf_gpio.h"
// #include "nrf_log.h"
// #include "nrf_log_ctrl.h"
// #include "nrf_log_default_backends.h"
// #include "nrf_pwr_mgmt.h"
// #include "nrf_drv_spi.h"

// #include "buckler.h"
// #include "display.h"
// #include "kobukiActuator.h"
// #include "kobukiSensorPoll.h"
// #include "kobukiSensorTypes.h"
// #include "kobukiUtilities.h"
// #include "lsm9ds1.h"
// #include "simple_ble.h"

// #include "states.h"

// // I2C manager
// NRF_TWI_MNGR_DEF(twi_mngr_instance, 5, 0);

// // global variables
// KobukiSensors_t sensors = {0};

// // Intervals for advertising and connections
// static simple_ble_config_t ble_config = {
//         // c0:98:e5:49:xx:xx
//         .platform_id       = 0x46,    // used as 4th octect in device BLE address
//         .device_id         = 0xEEC5, 
//         .adv_name          = "Robo-AR", // used in advertisements if there is room
//         .adv_interval      = MSEC_TO_UNITS(1000, UNIT_0_625_MS),
//         .min_conn_interval = MSEC_TO_UNITS(100, UNIT_1_25_MS),
//         .max_conn_interval = MSEC_TO_UNITS(200, UNIT_1_25_MS),
// };

// //4607eda0-f65e-4d59-a9ff-84420d87a4ca
// static simple_ble_service_t robot_service = {{
//     .uuid128 = {0xca,0xa4,0x87,0x0d,0x42,0x84,0xff,0xA9,
//                 0x59,0x4D,0x5e,0xf6,0xa0,0xed,0x07,0x46}
// }};

// //Declare characteristics and variables for your service
// static simple_ble_char_t waypoint_char = {.uuid16 = 0xeda1};
// static simple_ble_char_t ack_char = {.uuid16 = 0xeda2};
// simple_ble_app_t* simple_ble_app;

// float waypoint[2] = {0, 0};
// int acknowledged = 0;
// states state = OFF;
// //float total_distance = 0;
// float total_distance_left = 0;
// float total_distance_right = 0;
// bool connected = false;

// void readInput() {
//     printf("Bluetooth message recieved\n");
//     if (state == WAITING) {
//         printf("Distance: %f\n", waypoint[0]);
//         printf("Angle: %f\n", waypoint[1]);
//         acknowledged = 1;
// }

// void ble_evt_connected(ble_evt_t const* p_ble_evt) {
//     connected = true;
// }

// void ble_evt_disconnected(ble_evt_t const* p_ble_evt) {
//     connected = false;
// }


// void ble_evt_write(ble_evt_t const* p_ble_evt) {
//     //logic for each characteristic and related state changes
//     //Try not to modify the state here...
//     readInput();
// }

// void print_state(states current_state){
//     char buf[16];
// 	switch(current_state){
// 	case OFF: {
// 		display_write("OFF", DISPLAY_LINE_0);
//         snprintf(buf, 16, "", waypoint[1]);
// 		display_write(buf, DISPLAY_LINE_1);
// 		break;
//       }
// 	case WAITING: {
// 		display_write("WAITING", DISPLAY_LINE_0);
//         snprintf(buf, 16, "", waypoint[1]);
// 		display_write(buf, DISPLAY_LINE_1);
// 		break;
//     }
// 	case TURNING: {
//         snprintf(buf, 16, "TURN TARGET: %f", waypoint[1]);
//         display_write(buf, DISPLAY_LINE_0);
//         snprintf(buf, 16, "ANGLE CUR: %f", lsm9ds1_read_gyro_integration().z_axis);
// 		display_write(buf, DISPLAY_LINE_1);
// 		break;
//     }
// 	case DRIVING: {
//         snprintf(buf, 16, "TARGET: %.2f", waypoint[0]);
// 		display_write(buf, DISPLAY_LINE_0);
//         snprintf(buf, 16, "L: %.2f R: %.2f", total_distance_left, total_distance_right);
// 		display_write(buf, DISPLAY_LINE_1);
// 		break;
//     }
//    }
// }

// const float CONVERSION = 0.0006108;
// static float measure_distance(uint16_t current_encoder, uint16_t previous_encoder) {
//   float distance = CONVERSION * (current_encoder - previous_encoder);
//   return distance;
// }

// int main(void) {
//   ret_code_t error_code = NRF_SUCCESS;

//   // initialize RTT library
//   error_code = NRF_LOG_INIT(NULL);
//   APP_ERROR_CHECK(error_code);
//   NRF_LOG_DEFAULT_BACKENDS_INIT();
//   printf("Log initialized!\n");

//   // Setup BLE
//   simple_ble_app = simple_ble_init(&ble_config);

//   simple_ble_add_service(&robot_service);

//   //Register your characteristics
//   simple_ble_add_characteristic(0, 1, 0, 0,
//       sizeof(waypoint), (uint8_t*)&waypoint,
//       &robot_service, &waypoint_char);

//   simple_ble_add_characteristic(1, 0, 1, 0, 
//     sizeof(acknowledged), (uint8_t*)&acknowledged, 
//     &robot_service, &ack_char);
  
//   // Start Advertising
//   simple_ble_adv_only_name();

//   // initialize LEDs
//   nrf_gpio_pin_dir_set(23, NRF_GPIO_PIN_DIR_OUTPUT);
//   nrf_gpio_pin_dir_set(24, NRF_GPIO_PIN_DIR_OUTPUT);
//   nrf_gpio_pin_dir_set(25, NRF_GPIO_PIN_DIR_OUTPUT);

//   nrf_gpio_pin_set(23);
//   nrf_gpio_pin_set(24);
//   nrf_gpio_pin_set(25);



//   // initialize display
//   nrf_drv_spi_t spi_instance = NRF_DRV_SPI_INSTANCE(1);
//   nrf_drv_spi_config_t spi_config = {
//     .sck_pin = BUCKLER_LCD_SCLK,
//     .mosi_pin = BUCKLER_LCD_MOSI,
//     .miso_pin = BUCKLER_LCD_MISO,
//     .ss_pin = BUCKLER_LCD_CS,
//     .irq_priority = NRFX_SPI_DEFAULT_CONFIG_IRQ_PRIORITY,
//     .orc = 0,
//     .frequency = NRF_DRV_SPI_FREQ_4M,
//     .mode = NRF_DRV_SPI_MODE_2,
//     .bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST
//   };
//   error_code = nrf_drv_spi_init(&spi_instance, &spi_config, NULL, NULL);
//   APP_ERROR_CHECK(error_code);
//   display_init(&spi_instance);
//   printf("Display initialized!\n");

//   // initialize i2c master (two wire interface)
//   nrf_drv_twi_config_t i2c_config = NRF_DRV_TWI_DEFAULT_CONFIG;
//   i2c_config.scl = BUCKLER_SENSORS_SCL;
//   i2c_config.sda = BUCKLER_SENSORS_SDA;
//   i2c_config.frequency = NRF_TWIM_FREQ_100K;
//   error_code = nrf_twi_mngr_init(&twi_mngr_instance, &i2c_config);
//   APP_ERROR_CHECK(error_code);
//   lsm9ds1_init(&twi_mngr_instance);
//   printf("IMU initialized!\n");

//   // initialize Kobuki
//   kobukiInit();
//   printf("Kobuki initialized!\n");


//   float angle_threshold = .5;
//   float distance_threshold = .02;
//   float encoder_prev_left = 0;
//   float encoder_prev_right = 0;
//   float encoder_cur_left = 0;
//   float encoder_cur_right = 0;

//   // loop forever, running state machine
//   while (1) {
//     // read sensors from robot
//     int status = kobukiSensorPoll(&sensors);
//     // print out current state
//     print_state(state);
//     // switch into appropriate states
//     // OFF - No response to any signals until button pressed to go to WAITING
//     // WAITING - Do nothing until waypoint is recieved. Then go to TURNING
//     // Turning - Turn amount specified by waypoint. Then go DRIVING
//     // Driving - Drive the amount specified by waypoint. Then go back to WAITING
//     switch(state) {
//       case OFF: {
//         if (is_button_pressed(&sensors) || connected) {
//           nrf_gpio_pin_clear(23);
//           state = WAITING;
//         } else {
//           state = OFF;
//           kobukiDriveDirect(0, 0);
//         }
//         break;
//       }
//       case WAITING: {
//         if (is_button_pressed(&sensors) || !connected) {
//           state = OFF;
//         } else if (acknowledged==1) {
//             lsm9ds1_stop_gyro_integration();
//             lsm9ds1_start_gyro_integration();
//             state = TURNING;
//         } else {
//           state = WAITING;
//           kobukiDriveDirect(0, 0);
//         }
//         break;
//       }
//       case TURNING: {
//         float current_angle = lsm9ds1_read_gyro_integration().z_axis;
//         float diff = waypoint[1] - current_angle;
//         if (is_button_pressed(&sensors) || !connected) {
//           lsm9ds1_stop_gyro_integration();
//           state = OFF;
//         } else if (fabs(diff) < angle_threshold) {
//             lsm9ds1_stop_gyro_integration();
//             encoder_prev_left = sensors.leftWheelEncoder;
//             encoder_prev_right = sensors.rightWheelEncoder;
//             state = DRIVING;
//         }
//         else {
//           state = TURNING;
//           int8_t sign = (2 * (diff > 0)) - 1;
//           int16_t speed = sign * fmax(.8 * fabs(diff), 50);
//           kobukiDriveDirect(-speed, speed);
//         }
//         break;
//       }
//       case DRIVING: {
//         float diff_left = waypoint[0] - total_distance_left;
//         float diff_right = waypoint[0] - total_distance_right;
//         float wheel_diff = total_distance_left - total_distance_right;
//         if (is_button_pressed(&sensors) || !connected) {
//             total_distance_left = 0;
//             total_distance_right = 0;
//             //total_distance = 0;
//           state = OFF;
//         } else if ((fabs(diff_left) < distance_threshold) && (fabs(diff_right) < distance_threshold)) {
//             total_distance_left = 0;
//             total_distance_right = 0;
//             acknowledged = 0;
//             simple_ble_notify_char(&ack_char);
//             //total_distance = 0;
//             state = WAITING;
//         } else {
//           state = DRIVING;
//           encoder_cur_left = sensors.leftWheelEncoder;
//           encoder_cur_right = sensors.rightWheelEncoder;
//           total_distance_left += measure_distance(encoder_cur_left, encoder_prev_left);
//           total_distance_right += measure_distance(encoder_cur_right, encoder_prev_right);
//           //total_distance = (.5 * total_distance_left) + (.5 * total_distance_right);
//           encoder_prev_left = encoder_cur_left;
//           encoder_prev_right = encoder_cur_right;
//           int8_t sign_left = (2 * (diff_left > 0)) - 1;
//           int8_t sign_right = (2 * (diff_right > 0)) - 1;
//           //Wheel diff might not work if backwards :(
//           int k_dist = 190;
//           int k_diff = 250;
//           int16_t speed_left =  sign_left * fmax(fabs(k_dist * diff_left - k_diff * wheel_diff), 70);
//           int16_t speed_right =  sign_right * fmax(fabs(k_dist * diff_right + k_diff * wheel_diff), 70);
//           kobukiDriveDirect(speed_left, speed_right);
//         }
//         break;
//       }
//     }
//   }
// }

#include "states.h"
NRF_TWI_MNGR_DEF(twi_mngr_instance, 5, 0);


int main(void) {
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

  setup();
  system_state_t system_state = init_state();
  while(true) {
    inputs_t inputs = get_inputs();
    outputs_t output = transition(inputs, &system_state);
    display_write(output.display_line_0, DISPLAY_LINE_0);
    display_write(output.display_line_1, DISPLAY_LINE_1);
    do_outputs(output);
  }
}