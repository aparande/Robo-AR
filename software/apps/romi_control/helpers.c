#include "helpers.h"

// Set global Variables
KobukiSensors_t sensors = {0};
ret_code_t error_code = NRF_SUCCESS;
bool is_gyro_integrating = false;

// Intervals for advertising and connections
simple_ble_config_t ble_config = {
        // MAC address of form c0:98:e5:49:xx:xx, XX:XX is platform_id
        .platform_id       = 0x46,    // used as 4th octect in device BLE address
        .device_id         = 0xEEC5, 
        .adv_name          = "Robo-AR", // used in advertisements if there is room
        .adv_interval      = MSEC_TO_UNITS(1000, UNIT_0_625_MS),
        .min_conn_interval = MSEC_TO_UNITS(100, UNIT_1_25_MS),
        .max_conn_interval = MSEC_TO_UNITS(200, UNIT_1_25_MS),
};

// Initalize BLE service
simple_ble_service_t robot_service = {{
    .uuid128 = {0xca,0xa4,0x87,0x0d,0x42,0x84,0xff,0xA9,
                0x59,0x4D,0x5e,0xf6,0xa0,0xed,0x07,0x46}
}};

// Bluetooth Variables and Functions
float waypoint[2] = {0, 0};
bool new_waypoint_written = false;
int acknowledged = 0;
bool connected = false;

//Declare characteristics and variables for your service
simple_ble_char_t waypoint_char = {.uuid16 = 0xeda1};
simple_ble_char_t ack_char = {.uuid16 = 0xeda2};
simple_ble_app_t* simple_ble_app;


// called upon Connect
void ble_evt_connected(ble_evt_t const* p_ble_evt) {
    connected = true;
}

// called upon disconnect
void ble_evt_disconnected(ble_evt_t const* p_ble_evt) {
    connected = false;
    waypoint[0] = 0;
    waypoint[1] = 0;
    new_waypoint_written = false;
    acknowledged = 0;
}


// called upon a write
void ble_evt_write(ble_evt_t const* p_ble_evt) {
    //logic for each characteristic and related state changes
    //Try not to modify the state here...
    printf("Bluetooth message recieved\n");
    printf("Distance: %f\n", waypoint[0]);
    printf("Angle: %f\n", waypoint[1]);
    new_waypoint_written = true;
}

// Runs major setup code for the main loop. Sets up Bluetooth and LEDs
void setup() {
  // initialize RTT library
  error_code = NRF_LOG_INIT(NULL);
  APP_ERROR_CHECK(error_code);
  NRF_LOG_DEFAULT_BACKENDS_INIT();
  printf("Log initialized!\n");

  // Setup BLE
  simple_ble_app = simple_ble_init(&ble_config);
  simple_ble_add_service(&robot_service);

  //Register Waypoint Characteristic
  simple_ble_add_characteristic(0, 1, 0, 0,
      sizeof(waypoint), (uint8_t*)&waypoint,
      &robot_service, &waypoint_char);

  //Register Acknowledged Characteristic
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


// Util Functions

// Measures the distance traveled between two encoder ticks.
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


// Changes an angle to be within -180 and 180 degrees. 
float angle_modulo(float possible_angle){
  if(possible_angle > 0){
    return fmodf(possible_angle + 180, 360) - 180; 
  } else {
    return -1 * (fmodf((-1 * possible_angle) + 180, 360) - 180);
  }
}

