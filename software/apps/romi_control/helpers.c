#include "helpers.h"

//NRF_TWI_MNGR_DEF(twi_mngr_instance, 5, 0);


// Global Variables 

KobukiSensors_t sensors = {0};
ret_code_t error_code = NRF_SUCCESS;
bool is_gyro_integrating = false;

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
  return fabs(val);
}

float angle_modulo(float possible_angle){

  if(possible_angle > 0){
    return fmodf(possible_angle + 180, 360) - 180; 
  } else {
    return -1 * (fmodf((-1 * possible_angle) + 180, 360) - 180);
  }
}

void translation_control(system_state_t* curr_state, outputs_t* output, int dir) {
  float diff_left = curr_state->distance_to_travel - curr_state->substate.total_distance_traveled_left;
  float diff_right = curr_state->distance_to_travel - curr_state->substate.total_distance_traveled_right;
  
  float wheel_diff = diff_right - diff_left;
  int8_t sign_left = (2 * (diff_left > 0)) - 1;
  int8_t sign_right = (2 * (diff_right > 0)) - 1;
  
  float dist_traveled_left = measure_distance(input_state.left_encoder, curr_state->substate.previous_left_encoder);
  float dist_traveled_right = measure_distance(input_state.right_encoder, curr_state->substate.previous_right_encoder);
  curr_state->substate.total_distance_traveled_left += dist_traveled_left;
  curr_state->substate.total_distance_traveled_right += dist_traveled_right;

  float avg_dist = dir * (dist_traveled_right + dist_traveled_left) / 2;
  curr_state->position_x += cosf(curr_state->curr_orientation_angle * M_PI / 180) * avg_dist;
  curr_state->position_y += sinf(curr_state->curr_orientation_angle * M_PI / 180) * avg_dist;

  curr_state->substate.previous_left_encoder = input_state.left_encoder;
  curr_state->substate.previous_right_encoder = input_state.right_encoder;
  *output.left_speed = dir * sign_left * fmax(fabs(k_dist * diff_left - k_diff * wheel_diff), min_drive_speed);
  *output.right_speed = dir * sign_right * fmax(fabs(k_dist * diff_right + k_diff * wheel_diff), min_drive_speed);

}
