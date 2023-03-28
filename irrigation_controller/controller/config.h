#ifndef CONFIG_H
#define CONFIG_H

// Configuration defaults will be used if the config files are empty.
// There are two seperate config files to seperate network config 
// Network config can be reset over MQTT through the reset topic
// Other configs can be changed through the config topic

#define DEBUG true  //set to true for debug output, false for no debug output
#define DEBUG_OUT if(DEBUG)Serial


/*
Define what resevoirs the controller is connected to

1: Inexhaustible resevoir 
2: Exhaustible resevoir
3: Both
*/
#define RESEVOIR_MODE 3

/*
If using an inexhaustible resevoir, define the flow rate measurment method for the resevoir

true: Flow sensor
false: Static, pre-set flow rate
*/
#define USING_INEXHAUSTIBLE_FLOW true

/*
If using an exhaustible resevoir, define whether a drain valve is connected

true: A drain valve is connected to the resevoir
false: A drain valve is not connected to the resevoir
*/
#define USING_DRAIN_VALVE true


/*
If using an exhaustible resevoir, define whether a pressure sensor is being used

true: A pressure sensor is being used with a the resevoir
false: A pressure sensor is not being used with the resevoir
*/
#define USING_PRESSURE_SENSOR true

// ************* Pin config ************* //

#define INEXHAUSTIBLE_VALVE_PIN 13 // Pin to control the inexaustible resevoir supply valve
#define EXHAUSTIBLE_OUTPUT_VALVE_PIN 12 // Pin controlling the exhaustible resevoir supply valve
#define EXHAUSTIBLE_DRAIN_VALVE_PIN 14 // Pin controlling the exhaustible resevoir drain valve
#define FLOW_SENSOR_PIN 5 // Pin reading the flow sensor data output
#define PRESSURE_SENSOR_PIN 1 // Pin connected to pressure sensor I2C interface

// ************* WiFiManager AutoConnect config ************* //

#define AP_NAME "irrigation_controller" // Autoconnect access point name
#define AP_PASSWORD "verdantech" // Autoconnect access point password
#define AP_RETRY_DELAY 300000000 // Deep sleep delay between access point timeout and next try in microseconds
#define AP_IP IPAddress(192, 168, 0, 0) // Autoconnect access point IP
#define AP_GATEWAY IPAddress(192, 168, 0, 1) // Autoconnect access point gateway
#define AP_SUBNET IPAddress(255, 255, 255, 0) // Autoconnect access point subnet

// ************* MQTT network config ************* //

#define MQTT_SERVER_DOMAIN_DEFAULT "192.168.0.195" // Default MQTT server/broker URL
#define MQTT_SERVER_PORT_DEFAULT "1883" // Default MQTT server/broker port
#define MQTT_ID_DEFAULT "irrigation_controller1" // Default MQTT client ID
#define MQTT_USERNAME_DEFAULT "username" // Default MQTT username
#define MQTT_PASSWORD_DEFAULT "VerdanTech-Devices" // Default MQTT password
#define MQTT_RETRY_TIMEOUT_SECONDS 300 // MQTT connection timeout in seconds before return to AP config portal

// ************* MQTT topic config ************* //
#define BASE_TOPIC "irrigation_controller1/" // Base MQTT topic to be pre-fixed to all other topics
#define DISPENSE_ACTIVATE_TOPIC "dispense/activate" // Topic to subscribe to dispensation commands
#define DISPENSE_REPORT_TOPIC "dispense/report" // Topic to publish dispensation reports
#define DEACTIVATE_TOPIC "deactivate" // Topic to subscribe to deactivation commands
#define STATUS_TOPIC "status" // Topic to publish device status
#define CONFIG_TOPIC "config" // Topic to publish current configuration values  
#define CONFIG_CHANGE_TOPIC "config/change" // Topic to subscribe to configuration change commands

// The following topics are only used if using an exhaustible resevoir with a drain
#define DRAIN_ACTIVATE_TOPIC "drain/activate" // Topic to subscribe to drain commands
#define DRAIN_REPORT_TOPIC "drain/report" // Topic to publish drain reports

// The following topics are only used on configurations with a flow sensor
// Calibration should be implemented with MQTTv5 request/response and correlation data 
//#define CALIBRATE_FLOW_TOPIC "flow/calibrate"

// The following topics are only used on configurations with a pressure sensor
#define PRESSURE_REPORT_TOPIC "pressure/read" // Topic to publish current pressure sensor reading
//#define CALIBRATE_PRESSURE_TOPIC "pressure/calibrate "

// ************* Device config ************* //

// The amount of pulses per mililiter 
// returned by the flow sensor
#define PPL_DEFAULT 1265.289

// ************* Dispensation config ************* //

// The amount of miliseconds to wait for the 
// flow sensor to to sense flow before switching
// from the barrel to the hose
#define BARREL_TIMEOUT_MS_DEFAULT 5000

// The resolution of data in mililiters 
// returned after water dispensation
#define DATA_RESOLUTION_ML_DEFAULT 200

// ***************************************** //
// ************* Automatic configuration
// ************* don't change unless willing to modify code
// ****************************************** //

#define USING_INEXHAUSTIBLE_RESEVOIR_ ((RESEVOIR_MODE == 1 || RESEVOIR_MODE == 3) ? true : false)
#define USING_EXHAUSTIBLE_RESEVOIR_ ((RESEVOIR_MODE == 2 || RESEVOIR_MODE == 3) ? true : false)
#define USING_DRAIN_VALVE_ ((USING_EXHAUSTIBLE_RESEVOIR_ && USING_DRAIN_VALVE) ? true: false)
#define USING_FLOW_SENSOR_ ((USING_EXHAUSTIBLE_RESEVOIR_ || USING_INEXHAUSTIBLE_FLOW) ? true: false)
#define USING_PRESSURE_SENSOR_ ((USING_EXHAUSTIBLE_RESEVOIR_ && USING_PRESSURE_SENSOR) ? true: false)

// ************* Pins ************* //
// Define pins to -1 to indicate non-use based on operational mode
#define INEXHAUSTIBLE_SUPPLY_VALVE_PIN_ (USING_INEXHAUSTIBLE_RESEVOIR_ ? INEXHAUSTIBLE_SUPPLY_VALVE : -1)
#define EXHAUSTIBLE_SUPPLY_VALVE_PIN_ (USING_EXHAUSTIBLE_RESEVOIR_ ? EXHAUSTIBLE_SUPPLY_VALVE_PIN : -1)
#define DRAIN_VALVE_PIN_ (USING_DRAIN_VALVE_ ? DRAIN_VALVE_PIN : -1)
#define FLOW_SENSOR_PIN_ (USING_FLOW_SENSOR_ ? FLOW_SENSOR_PIN : -1)
#define PRESSURE_SENSOR_PIN_ (USING_PRESSURE_SENSOR_ ? PRESSURE_SENSOR_PIN : -1)

// ************* MQTT topics ************* //
// Define topics to -1 to indicate non-use based on operational mode
#define DISPENSE_ACTIVATE_TOPIC_ (BASE_TOPIC DISPENSE_ACTIVATE_TOPIC)
#define DISPENSE_REPORT_TOPIC_ (BASE_TOPIC DISPENSE_REPORT_TOPIC)
#define DEACTIVATE_TOPIC_ (BASE_TOPIC DEACTIVATE_TOPIC)
#define STATUS_TOPIC_ (BASE_TOPIC STATUS_TOPIC)
#define CONFIG_TOPIC_ (BASE_TOPIC CONFIG_TOPIC)
#define CONFIG_CHANGE_TOPIC_ (BASE_TOPIC CONFIG_CHANGE_TOPIC)
#define DRAIN_ACTIVATE_TOPIC_ (USING_DRAIN_VALVE_ ? (BASE_TOPIC DRAIN_ACTIVATE_TOPIC) : NULL)
#define DRAIN_REPORT_TOPIC_ (USING_DRAIN_VALVE_ ? (BASE_TOPIC DRAIN_REPORT_TOPIC) : NULL)
//#define CALIBRATE_FLOW_TOPIC_ (USING_FLOW_SENSOR_ ? (BASE_TOPIC CALIBRATE_FLOW_TOPIC) : NULL)
#define PRESSURE_REPORT_TOPIC_ (USING_PRESSURE_SENSOR_ ? (BASE_TOPIC PRESSURE_REPORT_TOPIC) : NULL)
//#define CALIBRATE_PRESSURE_TOPIC_ (USING_PRESSURE_SENSOR_ ? (BASE_TOPIC CALIBRATE_PRESSURE_TOPIC) : NULL)

#endif