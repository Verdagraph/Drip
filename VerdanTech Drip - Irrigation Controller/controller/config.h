// config.h
#ifndef CONFIG_H
#define CONFIG_H

#include <cstring>

#define DEBUG true  //set to true for debug output, false for no debug output
#define SLOG if(DEBUG)Serial

// *************
// ************* This group of settings are key configs that describe specific
// ************* physical and electrical assumptions about the device.
// ************* They enable/disable functions (mqtt topics, services)
// ************* Read the documentation before setting these!
// *************

/*
Define what resevoirs the controller is connected to

1: Source (Inexhaustible, constant or variable pressure water source) 
2: Tank (Exhaustible, variable pressure water tank)
3: Both
*/
#define RESEVOIR_MODE 1

/*
If using a source, define the flow rate measurment method for the resevoir

true: Flow sensor
false: Static, pre-set flow rate
*/
#define USING_SOURCE_FLOW false

/*
If using a tank, define whether a drain valve is connected

true: A drain valve is connected to the tank
false: A drain valve is not connected to the tank
*/
#define USING_DRAIN_VALVE true

/*
If using a tank, define whether a pressure sensor is being used

true: A pressure sensor is being used with a the tank
false: A pressure sensor is not being used with the tank
*/
#define USING_PRESSURE_SENSOR true

// *************
// ************* These settings are static and hard-coded
// ************* physical and electrical assumptions about the device.
// ************* They enable/disable functions (mqtt topics, services)
// ************* Read the documentation before setting these!
// *************

// ************* WiFiManager AutoConnect config ************* //

#define AP_NAME "irrigation_controller" // Autoconnect access point name
#define AP_PASSWORD "verdantech" // Autoconnect access point password
#define AP_IP IPAddress(192, 168, 0, 0) // Autoconnect access point IP
#define AP_GATEWAY IPAddress(192, 168, 0, 1) // Autoconnect access point gateway
#define AP_SUBNET IPAddress(255, 255, 255, 0) // Autoconnect access point subnet
#define AP_TIMEOUT // The amount of minutes to keep the autoconnect access point open before retry
#define AP_RETRY_DELAY 360 // Deep sleep delay between access point timeout and next try in seconds
#define AP_RETRY_DEEP_SLEEP true // Whether to use the ESP.deepSleep() function for low power or just delay()

// ************* MQTT network config ************* //

#define MQTT_SERVER_DOMAIN_DEFAULT "192.168.0.195" // Default MQTT server/broker URL
#define MQTT_SERVER_PORT_DEFAULT "1883" // Default MQTT server/broker port
#define MQTT_ID_DEFAULT "irrigation_controller1" // Default MQTT client ID
#define MQTT_USERNAME_DEFAULT "username" // Default MQTT username
#define MQTT_PASSWORD_DEFAULT "VerdanTech-Devices" // Default MQTT password
#define MQTT_RETRY_TIMEOUT 300 // MQTT connection timeout in seconds before return to AP config portal

// ************* Pin config ************* //

#define SOURCE_OUTPUT_VALVE_PIN 13 // Pin to control the source supply valve
#define TANK_OUTPUT_VALVE_PIN 12 // Pin controlling the tank supply valve
#define TANK_DRAIN_VALVE_PIN 14 // Pin controlling the tank drain valve
#define FLOW_SENSOR_PIN 5 // Pin reading the flow sensor data output

// *************
// ************* The following configs are defaults 
// ************* and can be changed during runtime 
// ************* through an MQTT topic
// *************

// The resolution of data in mililiters 
// returned after water dispensation
#define DATA_RESOLUTION_L_DEFAULT 0.2

// Define the static flow rate for the 
// source in liters/min
#define STATIC_FLOW_RATE_DEFAULT 12.45

// The amount of pulses per liter 
// returned by the flow sensor
#define PULSES_PER_L_DEFAULT 1265.289

// The max flow rate capable of being sensed
// by the flow sensor in liters/min
#define MAX_FLOW_RATE_DEFAULT 30

// The min flow rate capable of being sensed
// by the flow sensor in liters/min
// Resevoirs will switch at flow rates below this
#define MIN_FLOW_RATE_DEFAULT 0.2

// The amount of miliseconds to wait for the 
// flow sensor to sense flow before switching
// from the tank to the source
#define TANK_TIMEOUT_DEFAULT 5000

/*
Shape to use for tank height->volume calculations 

1: Rectangular prism (length, width, height)
2: Cylinder (radius, height, N/A)
*/
#define TANK_SHAPE_DEFAULT 2
#define TANK_DIMENSION_1_DEFAULT 0.4
#define TANK_DIMENSION_2_DEFAULT 1.2
#define TANK_DIMENSION_3_DEFAULT 0

/*
Select the output types of the pressure sensor

1: Pressure
2: Volume (requires exhaustible resevoir shape config or calibration)
3: Both
*/
#define PRESSURE_REPORT_MODE_DEFAULT 3

// The default 
#define ATMOSPHERIC_PRESSURE_HPA_DEFAULT 1013.25

// ************* MQTT topic config - Recommended to keep these short to save data ************* //
#define BASE_TOPIC "VD1/" // Base MQTT topic to be pre-fixed to all other topics
#define DISPENSE_ACTIVATE_TOPIC "out/on" // Topic to subscribe to dispensation commands
#define DISPENSE_REPORT_SLICE_TOPIC "out/log/sl" // Topic to publish slice of dispensation reports
#define DISPENSE_REPORT_SUMMARY_TOPIC "out/log/sm" // Topic to publish summary of dispensation reports
#define DEACTIVATE_TOPIC "off" // Topic to subscribe to deactivation commands
#define RESTART_TOPIC "restart" // Topic to restart the device
#define LOG_TOPIC "log/info" // Topic to publish device status logs
#define WARNING_TOPIC "log/warning" // Topic to publish device warning logs
#define ERROR_TOPIC "log/error" // Topic to publish device error logs
#define CONFIG_TOPIC "config" // Topic to publish current configuration values  
#define CONFIG_CHANGE_TOPIC "config/change" // Topic to subscribe to configuration change commands
#define SETTINGS_RESET_TOPIC "config/reset" // Topic to reset WiFi, MQTT, or config settings

// The following topics are only used if using an tank with a drain
#define DRAIN_ACTIVATE_TOPIC "drain/on" // Topic to subscribe to drain commands
#define DRAIN_REPORT_SUMMARY_TOPIC "drain/log" // Topic to publish drain reports

// The following topics are only used on configurations with a flow sensor
// Calibration should be implemented with MQTTv5 request/response and correlation data 
//#define CALIBRATE_FLOW_TOPIC "flow/calibrate"

// The following topics are only used on configurations with a pressure sensor
#define PRESSURE_REPORT_TOPIC "pressure/read" // Topic to publish current pressure sensor reading
//#define CALIBRATE_PRESSURE_TOPIC "pressure/calibrate "


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
#define CONFIG_CHANGE_TOPIC_ (BASE_TOPIC CONFIG_CHANGE_TOPIC)
#define SETTINGS_RESET_TOPIC_ (BASE_TOPIC SETTINGS_RESET_TOPIC)
#define DRAIN_ACTIVATE_TOPIC_ (USING_DRAIN_VALVE_ ? (BASE_TOPIC DRAIN_ACTIVATE_TOPIC) : NULL)
#define DRAIN_REPORT_SUMMARY_TOPIC_ (USING_DRAIN_VALVE_ ? (BASE_TOPIC DRAIN_REPORT_SUMMARY_TOPIC) : NULL)
//#define CALIBRATE_FLOW_TOPIC_ (USING_FLOW_SENSOR_ ? (BASE_TOPIC CALIBRATE_FLOW_TOPIC) : NULL)
#define PRESSURE_REPORT_TOPIC_ (USING_PRESSURE_SENSOR_ ? (BASE_TOPIC PRESSURE_REPORT_TOPIC) : NULL)
//#define CALIBRATE_PRESSURE_TOPIC_ (USING_PRESSURE_SENSOR_ ? (BASE_TOPIC CALIBRATE_PRESSURE_TOPIC) : NULL)

// ************* Useful constants ************* //
#define DENSITY_GRAVITY (9.80665 * 997.0474)
#define PI 3.1415926535897932384626433832795

namespace conf {

// ************* Config structs ************* //
struct MQTTConfig {
  char domain[100];
  char port[10];
  char id[50];
  char username[50];
  char password[50];

  MQTTConfig() {
    strcpy(domain, MQTT_SERVER_DOMAIN_DEFAULT);
    strcpy(port, MQTT_SERVER_PORT_DEFAULT);
    strcpy(id, MQTT_ID_DEFAULT);
    strcpy(username, MQTT_USERNAME_DEFAULT);
    strcpy(password, MQTT_PASSWORD_DEFAULT);
  }

};

struct ServicesConfig {
  int data_resolution_l;

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

  FlowSensorConfig() {
    pulses_per_l = PULSES_PER_L_DEFAULT;
    max_flow_rate = MAX_FLOW_RATE_DEFAULT;
    min_flow_rate = MIN_FLOW_RATE_DEFAULT;
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
