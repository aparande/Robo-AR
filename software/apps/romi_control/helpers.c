#include "helpers.h"

//NRF_TWI_MNGR_DEF(twi_mngr_instance, 5, 0);


// Global Variables 

KobukiSensors_t sensors = {0};
ret_code_t error_code = NRF_SUCCESS;


// Intervals for advertising and connections
simple_ble_config_t ble_config = {
        // c0:98:e5:49:xx:xx
        .platform_id       = 0x46,    // used as 4th octect in device BLE address
        .device_id         = 0xEEC5, 
        .adv_name          = "Robo-AR", // used in advertisements if there is room
        .adv_interval      = MSEC_TO_UNITS(1000, UNIT_0_625_MS),
        .min_conn_interval = MSEC_TO_UNITS(100, UNIT_1_25_MS),
        .max_conn_interval = MSEC_TO_UNITS(200, UNIT_1_25_MS),
};

//4607eda0-f65e-4d59-a9ff-84420d87a4ca
simple_ble_service_t robot_service = {{
    .uuid128 = {0xca,0xa4,0x87,0x0d,0x42,0x84,0xff,0xA9,
                0x59,0x4D,0x5e,0xf6,0xa0,0xed,0x07,0x46}
}};

//Declare characteristics and variables for your service
simple_ble_char_t waypoint_char = {.uuid16 = 0xeda1};
simple_ble_char_t ack_char = {.uuid16 = 0xeda2};
simple_ble_app_t* simple_ble_app;



const float CONVERSION = 0.0006108;
const float angle_threshold = .5;
const float distance_threshold = .02;
int k_dist = 210;
int k_diff = 250;
uint16_t min_angle_speed = 50;
uint16_t min_drive_speed = 80;


float waypoint[2] = {0, 0};
bool new_waypoint_written = false;
int acknowledged = 0;
bool connected = false;


void ble_evt_connected(ble_evt_t const* p_ble_evt) {
    connected = true;
}

void ble_evt_disconnected(ble_evt_t const* p_ble_evt) {
    connected = false;
}

// void read_input() {
//     printf("Bluetooth message recieved\n");
//     if (state == WAITING) {
//         printf("Distance: %f\n", waypoint[0]);
//         printf("Angle: %f\n", waypoint[1]);
//     }
// }

void ble_evt_write(ble_evt_t const* p_ble_evt) {
    //logic for each characteristic and related state changes
    //Try not to modify the state here...
    printf("Bluetooth message recieved\n");
	printf("Distance: %f\n", waypoint[0]);
	printf("Angle: %f\n", waypoint[1]);
    new_waypoint_written = true;
}

void setup() {
  // initialize RTT library
  error_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(error_code);
  NRF_LOG_DEFAULT_BACKENDS_INIT();
  printf("Log initialized!\n");

  // Setup BLE
  simple_ble_app = simple_ble_init(&ble_config);

  simple_ble_add_service(&robot_service);

  //Register your characteristics
  simple_ble_add_characteristic(0, 1, 0, 0,
      sizeof(waypoint), (uint8_t*)&waypoint,
      &robot_service, &waypoint_char);

  simple_ble_add_characteristic(1, 0, 1, 0, 
    sizeof(acknowledged), (uint8_t*)&acknowledged, 
    &robot_service, &ack_char);
  
  // Start Advertising
  simple_ble_adv_only_name();

  // initialize LEDs
  nrf_gpio_pin_dir_set(23, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(24, NRF_GPIO_PIN_DIR_OUTPUT);
  nrf_gpio_pin_dir_set(25, NRF_GPIO_PIN_DIR_OUTPUT);

  nrf_gpio_pin_set(23);
  nrf_gpio_pin_set(24);
  nrf_gpio_pin_set(25);
}

float measure_distance(uint16_t current_encoder, uint16_t previous_encoder) {
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