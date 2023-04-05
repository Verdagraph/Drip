# VerdanTech Drip - Open Source Drip Irrigation Controller

The VDT Drip is an open source irrigation controller based on the ESP8266. The Drip's main function is to dispense a given volume of water with irrigation systems that integrate variable-flow water supplies.

To do this, the Drip employs 1-3 solenoid valve(s), 0-1 flow sensor(s), 0-1 pressure sensor(s), and communication via an MQTT broker.

## Get started guide

1. Read the terminology and MQTT interface to learn the devices capabilities
1. read the hardware sections Decide on a configuration and source materials
2. Assemble using soldering or another technique and all the pipes connected and stuff
3. Find some sort of MQTT broker
4. Download and install Arduino
5. Install required libraries
6. Configure config according to section (include distinctions between required configs and general defaults)
7. Upload code to ESP8266
8. Connect to WiFi and MQTT through autoconnect portal
9. Calibrate sensors

## Terminology

### Water Supplies

- **Source**
    - A supply of water that is assumed to be inexhaustible with a static or variable flow rate, such as a utility water supply.
- **Tank**
    - A supply of water that is assumed to be exhaustible with a variable flow rate, such as a rain barrel.

### Equipment

- **Valve**
    - A solenoid valve
- **Flow sensor**
    - A hall effect flow sensor
- **Pressure sensor**
    - I2C pressure sensor

### Software

- **Broker**
    - An MQTT broker
- **Topic**
    - An MQTT topic

## MQTT interface

| Title   | Topic string  | Does the controller publish or subscribe? | Is this functionality conditional on the operating mode? |
| ------------- | ------------- | ------------- | ------------- |
| Dispense activate  | `DISPENSE_ACTIVATE_TOPIC_`  | Subscribe | No |
| Dispense slice report  | `DISPENSE_REPORT_SLICE_TOPIC_`  | Publish | No |
| Dispense summary report  | `DISPENSE_REPORT_SUMMARY_TOPIC_`  | Publish | No |
| Deactivate  | `DEACTIVATE_TOPIC_`  | Subscribe | No |
| Restart  | `RESTART_TOPIC_`  | Subscribe | No |
| Info logs  | `LOG_TOPIC_`  | Publish | No |
| Warning logs  | `WARNING_TOPIC_`  | Publish | No |
| Error logs  | `ERROR_TOPIC_`  | Publish | No |
| Read config  | `CONFIG_TOPIC_`  | Publish | No |
| Write config  | `CONFIG_CHANGE_TOPIC_`  | Subscribe | No |
| Reset settings  | `SETTINGS_RESET_TOPIC_`  | Subscribe | No |
| Drain activate  | `DRAIN_ACTIVATE_TOPIC_`  | Subscribe | Yes |
| Drain summary report  | `DRAIN_REPORT_SUMMARY_TOPIC_`  | Publish | Yes |

### Dispensation

#### Activation




#### Reporting

##### Slices

##### Summaries

### Deactivation

### Restart

### Logging

#### Info

#### Warnings

#### Errors

### Config

#### Read

#### Write

### Reset Settings

### Drain

#### Activation

#### Reporting

# Hardware

## Operating Modes

## Required equipment

## 3D printed CAD files

## Assembly

# Software configuration

## Configuration

All configuration settings can be found in the file `config.h`.

### Core

These settings define the [operating mode](#operating-modes) of the device. Make sure you have the correct hardware connected before you enable any related settings here.

- `RESEVOIR_MODE int` Defines which [types of water supplies](#water-supplies) the controller is connected to.
    - `1` Source
    - `2` Tank
    - `3` Both

- `USING_SOURCE_FLOW bool` If using a source, define the method of flow rate estimation. 
    - `true` A flow sensor measures the flow rate. This is useful if the source has a variable flow, or if a tank and flow sensor is already being used.
    - `false` A flow sensor is not 

-  `USING_DRAIN_VALVE bool`
    - `true`
    - `false`

- `USING_PRESSURE_SENSOR bool`
    - `true`
    - `false`

### WiFi Manager

These settings are used to configure the WiFi Manager, an instance of the WiFiManager (see [dependencies](#dependencies)) class. This class enables the configuration of the WiFi network and MQTT broker through an access point hosted by the controller, rather than being hard coded into the software.

- `AP_NAME string`
- `AP_PASSWORD string`
- `AP_RETRY_DELAY int`
- `AP_IP IPAddress()`
- `AP_GATEWAY IPAddress()`
- `AP_SUBNET IPAddress()`

### MQTT

These settings are used to configue the MQTT client, an instance of the PubSubClient (see [dependencies](#dependencies)) class. This class enables interaction with an MQTT broker. The settings marked as `DEFAULT` are able to be configured through the WiFi Manager access point.

- `MQTT_SERVER_DOMAIN_DEFAULT string`
- `MQTT_SERVER_PORT_DEFAULT string`
- `MQTT_ID_DEFAULT string`
- `MQTT_USERNAME_DEFAULT string`
- `MQTT_PASSWORD_DEFAULT string`
- `MQTT_RETRY_TIMEOUT_SECONDS int`

### Pins

These settings define the pins used to interact with the equipment connected to the controller. Don't worry about changing the settings that don't apply to your use case - unused pins are automatically set to -1 in the [auto-config](#auto-config). 

- `SOURCE_OUTPUT_VALVE_PIN int`
- `TANK_OUTPUT_VALVE_PIN int`
- `TANK_DRAIN_VALVE_PIN int`
- `FLOW_SENSOR_PIN int`
- `PRESSURE_SENSOR_PIN int`

### Defaults

These settings are default settings that can be changed at runtime through the [config change topic](#write).

- `DATA_RESOLUTION_L_DEFAULT float`
- `STATIC_FLOW_RATE_DEFAULT float`
- `PULSES_PER_L_DEFAULT float`
- `MAX_FLOW_RATE_DEFAULT float`
- `MIN_FLOW_RATE_DEFAULT float`
- `TANK_TIMEOUT_DEFAULT int`
- `TANK_SHAPE_DEFAULT int`
    - `1`
    - `2`
- `TANK_DIMENSION_1_DEFAULT float`
- `TANK_DIMENSION_2_DEFAULT float`
- `TANK_DIMENSION_3_DEFAULT float`
- `PRESSURE_REPORT_MODE_DEFAULT int`
    - `1`
    - `2`
    - `3`
- `ATMOSPHERIC_PRESSURE_HPA_DEFAULT float`

### Topics

These settings define the topic strings to use for the MQTT interface. See the [MQTT interface](#mqtt-interface) for details.

You can leave these unchanged with no problem, but you might want to customize them. If you do, just keep in mind that the topic strings are sent with every message, and that the MQTT client has a maximum message size of 256 bytes (including header). Try to keep them small, especially for topics with a high message frequency (like `DISPENSE_REPORT_SLICE_TOPIC_`.)

- `BASE_TOPIC string` The base topic used to pre-pend all other topics through the [auto-config](#auto-config).
- `DISPENSE_ACTIVATE_TOPIC string`
- `DISPENSE_REPORT_SLICE_TOPIC string`
- `DISPENSE_REPORT_SUMMARY_TOPIC string`
- `DEACTIVATE_TOPIC string`
- `RESTART_TOPIC string`
- `LOG_TOPIC string`
- `WARNING_TOPIC string`
- `ERROR_TOPIC string`
- `CONFIG_TOPIC string`
- `CONFIG_CHANGE_TOPIC string`
- `SETTINGS_RESET_TOPIC string`
- `DRAIN_ACTIVATE_TOPIC string`
- `DRAIN_REPORT_SUMMARY_TOPIC string`

### Auto-Config

These settings are configured automatically and shouldn't be altered unless you're willing to change other parts of the code. They're self explanatory, but listed here for reference.

```
// ************* Operating mode ************* //
#define USING_SOURCE_ ((RESEVOIR_MODE == 1 || RESEVOIR_MODE == 3) ? true : false)
#define USING_TANK_ ((RESEVOIR_MODE == 2 || RESEVOIR_MODE == 3) ? true : false)
#define USING_DRAIN_VALVE_ ((USING_TANK_ && USING_DRAIN_VALVE) ? true: false)
#define USING_FLOW_SENSOR_ ((USING_TANK_ || USING_SOURCE_FLOW) ? true: false)
#define USING_PRESSURE_SENSOR_ ((USING_TANK_ && USING_PRESSURE_SENSOR) ? true: false)

// ************* Pins ************* //
// Define pins to -1 to indicate non-use based on operational mode
#define SOURCE_OUTPUT_VALVE_PIN_ (USING_SOURCE_ ? SOURCE_OUTPUT_VALVE_PIN : -1)
#define TANK_OUTPUT_VALVE_PIN_ (USING_TANK_ ? TANK_OUTPUT_VALVE_PIN : -1)
#define TANK_DRAIN_VALVE_PIN_ (USING_DRAIN_VALVE_ ? TANK_DRAIN_VALVE_PIN : -1)
#define FLOW_SENSOR_PIN_ (USING_FLOW_SENSOR_ ? FLOW_SENSOR_PIN : -1)
#define PRESSURE_SENSOR_PIN_ (USING_PRESSURE_SENSOR_ ? PRESSURE_SENSOR_PIN : -1)

// ************* MQTT topics ************* //
// Define topics to NULL to indicate non-use based on operational mode
#define DISPENSE_ACTIVATE_TOPIC_ (BASE_TOPIC DISPENSE_ACTIVATE_TOPIC)
#define DISPENSE_REPORT_SLICE_TOPIC_ (BASE_TOPIC DISPENSE_REPORT_SLICE_TOPIC)
#define DISPENSE_REPORT_SUMMARY_TOPIC_ (BASE_TOPIC DISPENSE_REPORT_SUMMARY_TOPIC)
#define DEACTIVATE_TOPIC_ (BASE_TOPIC DEACTIVATE_TOPIC)
#define RESTART_TOPIC_ (BASE_TOPIC RESTART_TOPIC)
#define LOG_TOPIC_ (BASE_TOPIC LOG_TOPIC)
#define ERROR_TOPIC_ (BASE_TOPIC ERROR_TOPIC)
#define WARNING_TOPIC_ (BASE_TOPIC WARNING_TOPIC)
#define CONFIG_TOPIC_ (BASE_TOPIC CONFIG_TOPIC)
#define CONFIG_CHANGE_TOPIC_ (BASE_TOPIC CONFIG_CHANGE_TOPIC)
#define SETTINGS_RESET_TOPIC_ (BASE_TOPIC SETTINGS_RESET_TOPIC)
#define DRAIN_ACTIVATE_TOPIC_ (USING_DRAIN_VALVE_ ? (BASE_TOPIC DRAIN_ACTIVATE_TOPIC) : NULL)
#define DRAIN_REPORT_SUMMARY_TOPIC_ (USING_DRAIN_VALVE_ ? (BASE_TOPIC DRAIN_REPORT_SUMMARY_TOPIC) : NULL)
```


## Dependencies

## For Contributors

### Explanation of the purposes of different files

### Explanation of app state 


# Bug reporting and contributing

Required libraries: 
WiFi manager: https://github.com/tzapu/WiFiManager Version: 2.0.15-rc.1
