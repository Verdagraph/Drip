// config.h
#ifndef CONFIG_H
#define CONFIG_H

#include <cstring>

// *************
// ************* See the documentation for detailed
// ************* descriptions about what each of the 
// ************* config settings does!
// *************


// ************* Core ************* //
// *************
// ************* This group of settings are key configs that describe specific
// ************* physical and electrical assumptions about the device.
// ************* They enable/disable functions (mqtt topics, services)
// ************* Read the documentation before setting these!
// *************

#define DEBUG true
#define SLOG if(DEBUG)Serial

#define RESEVOIR_MODE 1
#define USING_SOURCE_FLOW true
#define USING_DRAIN_VALVE true
#define USING_PRESSURE_SENSOR false


// ************* WiFiManager AutoConnect config ************* //

#define AP_NAME "irrigation_controller"
#define AP_PASSWORD "verdantech"
#define AP_IP IPAddress(192, 168, 0, 0)
#define AP_GATEWAY IPAddress(192, 168, 0, 1)
#define AP_SUBNET IPAddress(255, 255, 255, 0)
#define AP_TIMEOUT 120
#define AP_RETRY_DELAY 360
#define AP_RETRY_DEEP_SLEEP true


// ************* MQTT network config ************* //

#define MQTT_SERVER_DOMAIN_DEFAULT "192.168.0.195"
#define MQTT_SERVER_PORT_DEFAULT "1883"
#define MQTT_ID_DEFAULT "irrigation_controller1"
#define MQTT_USE_CREDENTIALS false
#define MQTT_USERNAME_DEFAULT "irrigation"
#define MQTT_PASSWORD_DEFAULT "tank"
#define MQTT_RETRY_TIMEOUT 300
#define MQTT_MAX_BUFFER_SIZE 512
#define MQTT_KEEPALIVE 15


// ************* Pin config ************* //

#define SOURCE_OUTPUT_VALVE_PIN 14
#define TANK_OUTPUT_VALVE_PIN 15
#define TANK_DRAIN_VALVE_PIN 13
#define FLOW_SENSOR_PIN 12


// ************* Defaults ************* //
// *************
// ************* The following configs are defaults 
// ************* and can be changed during runtime 
// ************* through an MQTT topic
// *************

#define DATA_RESOLUTION_L_DEFAULT 0.2
#define STATIC_FLOW_RATE_DEFAULT 12.45
#define PULSES_PER_L_DEFAULT 1265.289
#define MAX_FLOW_RATE_DEFAULT 30
#define MIN_FLOW_RATE_DEFAULT 0.2
#define FLOW_CALIBRATION_TIMEOUT 30
#define FLOW_CALIBRATION_MAX_VOLUME 0.5
#define TANK_TIMEOUT_DEFAULT 10
#define TANK_SHAPE_DEFAULT 2
#define TANK_DIMENSION_1_DEFAULT 0.4
#define TANK_DIMENSION_2_DEFAULT 1.2
#define TANK_DIMENSION_3_DEFAULT 0
#define PRESSURE_REPORT_MODE_DEFAULT 3
#define ATMOSPHERIC_PRESSURE_HPA_DEFAULT 1013.25


// ************* MQTT topic config ************* //

#define BASE_TOPIC "VD1/"
#define DISPENSE_ACTIVATE_TOPIC "out/on"
#define DISPENSE_REPORT_SLICE_TOPIC "out/log/sl"
#define DISPENSE_REPORT_SUMMARY_TOPIC "out/log/sm"
#define DEACTIVATE_TOPIC "off"
#define RESTART_TOPIC "restart"
#define LOG_TOPIC "log/info"
#define WARNING_TOPIC "log/warning"
#define ERROR_TOPIC "log/error"
#define CONFIG_TOPIC "config"
#define AUTO_CONFIG_TOPIC "config/auto" 
#define TOPIC_CONFIG_TOPIC "topics"
#define CONFIG_CHANGE_TOPIC "config/change"
#define SETTINGS_RESET_TOPIC "config/reset"
#define FLOW_SENSOR_CALIBRATE_BEGIN_TOPIC "flow/calibrate"
#define FLOW_SENSOR_CALIBRATE_DISPENSE_TOPIC "flow/dispense"
#define FLOW_SENSOR_CALIBRATE_MEASURE_TOPIC "flow/measure"
#define DRAIN_ACTIVATE_TOPIC "drain/on"
#define DRAIN_REPORT_SUMMARY_TOPIC "drain/log"
#define PRESSURE_REQUEST_TOPIC "pressure/request"
#define PRESSURE_REPORT_TOPIC "pressure/report"


// ************* Auto-config ************* //
// *************
// ************* This group of settings are automatically configured.
// ************* Don't change unless willing to modify code!
// *************

// ************* Operating mode ************* //
#define USING_SOURCE_ ((RESEVOIR_MODE == 1 || RESEVOIR_MODE == 3) ? true : false)
#define USING_TANK_ ((RESEVOIR_MODE == 2 || RESEVOIR_MODE == 3) ? true : false)
#define USING_DRAIN_VALVE_ ((USING_TANK_ && USING_DRAIN_VALVE) ? true : false)
#define USING_FLOW_SENSOR_ ((USING_TANK_ || USING_SOURCE_FLOW) ? true : false)
#define USING_PRESSURE_SENSOR_ ((USING_TANK_ && USING_PRESSURE_SENSOR) ? true : false)

// ************* Pins ************* //
// Define pins to -1 to indicate non-use based on operational mode
#define SOURCE_OUTPUT_VALVE_PIN_ (USING_SOURCE_ ? SOURCE_OUTPUT_VALVE_PIN : -1)
#define TANK_OUTPUT_VALVE_PIN_ (USING_TANK_ ? TANK_OUTPUT_VALVE_PIN : -1)
#define TANK_DRAIN_VALVE_PIN_ (USING_DRAIN_VALVE_ ? TANK_DRAIN_VALVE_PIN : -1)
#define FLOW_SENSOR_PIN_ (USING_FLOW_SENSOR_ ? FLOW_SENSOR_PIN : -1)

// ************* MQTT topics ************* //
// Define topics to NULL to indicate non-use based on operational mode
#define DISPENSE_ACTIVATE_TOPIC_ (BASE_TOPIC DISPENSE_ACTIVATE_TOPIC)
#define DISPENSE_REPORT_SLICE_TOPIC_ (BASE_TOPIC DISPENSE_REPORT_SLICE_TOPIC)
#define DISPENSE_REPORT_SUMMARY_TOPIC_ (BASE_TOPIC DISPENSE_REPORT_SUMMARY_TOPIC)
#define DEACTIVATE_TOPIC_ (BASE_TOPIC DEACTIVATE_TOPIC)
#define RESTART_TOPIC_ (BASE_TOPIC RESTART_TOPIC)
#define LOG_TOPIC_ (BASE_TOPIC LOG_TOPIC)
#define WARNING_TOPIC_ (BASE_TOPIC WARNING_TOPIC)
#define ERROR_TOPIC_ (BASE_TOPIC ERROR_TOPIC)
#define CONFIG_TOPIC_ (BASE_TOPIC CONFIG_TOPIC)
#define AUTO_CONFIG_TOPIC_ (BASE_TOPIC AUTO_CONFIG_TOPIC)
#define TOPIC_CONFIG_TOPIC_ (BASE_TOPIC TOPIC_CONFIG_TOPIC)
#define CONFIG_CHANGE_TOPIC_ (BASE_TOPIC CONFIG_CHANGE_TOPIC)
#define SETTINGS_RESET_TOPIC_ (BASE_TOPIC SETTINGS_RESET_TOPIC)
#define FLOW_SENSOR_CALIBRATE_BEGIN_TOPIC_ (USING_FLOW_SENSOR_ ? (BASE_TOPIC FLOW_SENSOR_CALIBRATE_BEGIN_TOPIC) : NULL)
#define FLOW_SENSOR_CALIBRATE_DISPENSE_TOPIC_ (USING_FLOW_SENSOR_ ? (BASE_TOPIC FLOW_SENSOR_CALIBRATE_DISPENSE_TOPIC) : NULL)
#define FLOW_SENSOR_CALIBRATE_MEASURE_TOPIC_ (USING_FLOW_SENSOR_ ? (BASE_TOPIC FLOW_SENSOR_CALIBRATE_MEASURE_TOPIC) : NULL)
#define DRAIN_ACTIVATE_TOPIC_ (USING_DRAIN_VALVE_ ? (BASE_TOPIC DRAIN_ACTIVATE_TOPIC) : NULL)
#define DRAIN_REPORT_SUMMARY_TOPIC_ (USING_DRAIN_VALVE_ ? (BASE_TOPIC DRAIN_REPORT_SUMMARY_TOPIC) : NULL)
#define PRESSURE_REQUEST_TOPIC_ (USING_PRESSURE_SENSOR_ ? (BASE_TOPIC PRESSURE_REQUEST_TOPIC) : NULL)
#define PRESSURE_REPORT_TOPIC_ (USING_PRESSURE_SENSOR_ ? (BASE_TOPIC PRESSURE_REPORT_TOPIC) : NULL)

// ************* Useful constants ************* //
#define DENSITY_GRAVITY (9.80665 * 997.0474)
#define PI 3.1415926535897932384626433832795

namespace conf {

// ************* Runtime config structs ************* //
struct MQTTConfig {
  char domain[100];
  char port[10];
  char id[50];
  char username[50];
  char password[50];

  MQTTConfig() {
    strlcpy(domain, MQTT_SERVER_DOMAIN_DEFAULT, 100);
    strlcpy(port, MQTT_SERVER_PORT_DEFAULT, 10);
    strlcpy(id, MQTT_ID_DEFAULT, 50);
    strlcpy(username, MQTT_USERNAME_DEFAULT, 50);
    strlcpy(password, MQTT_PASSWORD_DEFAULT, 50);
  }

};

struct ServicesConfig {
  float data_resolution_l;

  ServicesConfig() {
    data_resolution_l = DATA_RESOLUTION_L_DEFAULT;
  }
};

struct SourceConfig {
  float static_flow_rate;
  SourceConfig() {
    static_flow_rate = STATIC_FLOW_RATE_DEFAULT;
  }
};

struct TankConfig {
  int tank_timeout;

  int shape_type;
  float dimension_1;
  float dimension_2;
  float dimension_3;

  TankConfig () {
    tank_timeout = TANK_TIMEOUT_DEFAULT;
    shape_type = TANK_SHAPE_DEFAULT;
    dimension_1 = TANK_DIMENSION_1_DEFAULT;
    dimension_2 = TANK_DIMENSION_2_DEFAULT;
    dimension_3 = TANK_DIMENSION_3_DEFAULT;
  }

};

struct FlowSensorConfig {
  float pulses_per_l;
  float max_flow_rate;
  float min_flow_rate;
  int calibration_timeout;
  float calibration_max_volume;

  FlowSensorConfig() {
    pulses_per_l = PULSES_PER_L_DEFAULT;
    max_flow_rate = MAX_FLOW_RATE_DEFAULT;
    min_flow_rate = MIN_FLOW_RATE_DEFAULT;
    calibration_timeout = FLOW_CALIBRATION_TIMEOUT;
    calibration_max_volume = FLOW_CALIBRATION_MAX_VOLUME;
  }
};

struct PressureSensorConfig {
  int report_mode;
  float atmosphere_pressure;

  PressureSensorConfig() {
    report_mode = PRESSURE_REPORT_MODE_DEFAULT;
    atmosphere_pressure = ATMOSPHERIC_PRESSURE_HPA_DEFAULT;
  }

};

}

#endif
