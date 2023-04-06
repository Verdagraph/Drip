# VerdanTech Drip - Open Source Drip Irrigation Controller

The VDT Drip is an open source irrigation controller based on the ESP8266. The Drip's main function is to dispense a given volume of water with irrigation systems that integrate variable-flow water supplies.

To do this, the Drip employs 1-3 solenoid valve(s), 0-1 flow sensor(s), 0-1 pressure sensor(s), and communication via an MQTT broker.

## Project Instructions

1. Read the [terminology](#terminology) and [interface](#mqtt-interface) sections 
to understand the device's capabilities and limitations.

2. Read the [hardware](#hardware) section and decide on an operating mode.

3. Secure the use of an MQTT broker

4. Source materials

5. Assemble the device

6. Install the Arduino platform and required libraries

7. Configure the device through the file `config.h`

8. Flash the microcontroller with the software

9. Connect to a WiFi network and MQTT broker through the auto-connect access point

10. Calibrate sensors

## Terminology

Refer to this list for explicit descriptions of some key terms used throughout the document.

### Water Supplies

- **Source**
    - A supply of water that is assumed to be inexhaustible with a static or variable flow rate, such as a utility water supply.
- **Tank**
    - A supply of water that is assumed to be exhaustible with a variable flow rate, such as a rain barrel.

### Equipment

For specific product examples see [equipment](#recommended-equipment).

- **Valve**
    - An electronically controllable water valve, such as a solenoid valve.
- **Flow sensor**
    - A water flow sensor capable of sending a steady rate of electrical pulses per unit volume, such as a hall effect flow sensor.
- **Pressure sensor**
    - A pressure sensor capable of sending an absolute pressure reading through I2C and of surviving the maximum water pressure in the **Tank**.
- **Operational mode**
    - A specific combination of source, tank, valves, flow sensor, and pressure sensor, as described in the [hardware](#operating-modes) section.

### Software

- **MQTT**
    - A network protocol used for internet-of-things (IOT) devices. See [wiki](https://en.wikipedia.org/wiki/MQTT).

- **WiFi Manager**
    - An instance of the WiFiManager class. See [library](#dependencies).

- **MQTT Client**
    - An instance of the PubSubClient class. See [library](#dependencies). 

### Operation

- **Dispensation**
    - A process of attempting to output a target volume of water, first from the tank, then from the source.

## MQTT interface

This section describes the functionality of the MQTT topics through which the device sends and recieves information. All communication is done via JSON.

| Title   | Topic string config  | Does the controller publish or subscribe? | Is the topic enabled unconditionally? |
| ------------- | ------------- | ------------- | ------------- |
| [Dispense Activate](#activation)  | `DISPENSE_ACTIVATE_TOPIC_` | Subscribe | Yes |
| [Dispense Slice Report](#slice-reporting)  | `DISPENSE_REPORT_SLICE_TOPIC_`  | Publish | Yes |
| [Dispense Summary Report](#summary-reporting)  | `DISPENSE_REPORT_SUMMARY_TOPIC_`  | Publish | Yes |
| [Deactivate](#deactivation)  | `DEACTIVATE_TOPIC_`  | Subscribe | Yes |
| [Restart](#restart)  | `RESTART_TOPIC_`  | Subscribe | Yes |
| [Info Logging](#info)  | `LOG_TOPIC_`  | Publish | Yes |
| [Warning Logging](#warnings)  | `WARNING_TOPIC_`  | Publish | Yes |
| [Error Logging](#errors)  | `ERROR_TOPIC_`  | Publish | Yes |
| [Read Config](#read)  | `CONFIG_TOPIC_`  | Publish | Yes |
| [Write Config](#write)  | `CONFIG_CHANGE_TOPIC_`  | Subscribe | Yes |
| [Reset Settings](#reset-settings)  | `SETTINGS_RESET_TOPIC_`  | Subscribe | Yes |
| [Drain Activate](#activation-1)  | `DRAIN_ACTIVATE_TOPIC_`  | Subscribe | No |
| [Drain Summary Report](#reporting)  | `DRAIN_REPORT_SUMMARY_TOPIC_`  | Publish | No |

### Dispensation

The main functionality of the device. The dispentation topics handle setting target volumes for the dispensation process, publishing data taken at regular intervals through the process, and publishing data at the end of the process.

#### Activation

| Topic string config  | Publish or subscribe? | Enabled unconditionally? |
| ------------- | ------------- | ------------- |
| `DISPENSE_ACTIVATE_TOPIC_`  | Subscribe | Yes |

This topic allows activating the dispensation process and setting the target volume. Activation will fail if a dispensation process or drain process is already in progress. The dispensation process will continue until the output volume is greater than or equal to the target volume, or a singularly connected tank produces a flow rate less than that defined by `MIN_FLOW_RATE_DEFAULT` and the total duration of the process is greater than that defined by `TANK_TIMEOUT_DEFAULT`.

Inputs:
- `["tv"]` float. Target volume of the dispensation process in liters.

Sample payload:
```
{
  "tv": 10.25
}
```

#### Slice Reporting

| Topic string config  | Publish or subscribe? | Enabled unconditionally? |
| ------------- | ------------- | ------------- | 
| `DISPENSE_REPORT_SLICE_TOPIC_`  | Publish | Yes |

This topic is where the mid-process datapoints on the dispensation process are published between a volume interval in liters defined by `DATA_RESOLUTION_L_DEFAULT`. If the connection to the MQTT broker fails, these reports will fail to be sent and are lost, but the dispensation process will continue.

Outputs:
- `t` int. Current duration of the dispensation process in miliseconds.
- `v` float. Current output volume of the dispensation process in liters.
- `q` float. The average flow rate of output volume calculated either from the last slice report to the current one (or from the beginning of the process if on the first report) in liters per minute
- `tp` (conditional) float. The average tank gauge pressure calculated from the last slice report to the current one (or from the beginning of the process if on the first report) in hectopascals. Enabled if `USING_PRESSURE_SENSOR_` and `PRESSURE_REPORT_MODE_DEFAULT` equals 1 or 3.
- `tv` (conditional) float. The average tank volume calculated from the last slice report to the current one (or from the beginning of the process if on the first report) in liters. Enabled if `USING_PRESSURE_SENSOR_` and `PRESSURE_REPORT_MODE_DEFAULT` equals 2 or 3.

Sample payload:
```
{
  "t": 5000,
  "v": 0.1,
  "q": 13.2,
  "tp": 68.94,
  "tv": 85.6
}
```

#### Summary Reporting

| Topic string config  | Publish or subscribe? | Enabled unconditionally? |
| ------------- | ------------- | ------------- |
| `DISPENSE_REPORT_SUMMARY_TOPIC_`  | Publish | Yes |

This topic is where the post-process datapoints on the dispensation process are published. If the connection to the MQTT broker fails, the connection is retried once before the message is lost.

Outputs:
- `tt` int. Total duration of the dispensation process in miliseconds.
- `vt` float. Total output volume of the dispensation process in liters.
- `tv` (conditional) float. The total output volume produced by the tank in liters. Enabled if `USING_TANK_`.
- `tts` (conditional) int. The time stamp at which the dispensation process switched from the tank to the source. Will equal `tt` if the processes ended without switching water supplies. Enabled if `USING_TANK_` and `USING_SOURCE_`.

Sample payload:
```
{
  "tt": 500000,
  "vt": 10.25,
  "tv": 5.6,
  "tts": 250000,
}
```

### Deactivation

| Topic string config  | Publish or subscribe? | Enabled unconditionally? |
| ------------- | ------------- | ------------- |
| `DEACTIVATE_TOPIC_`  | Subscribe | Yes |

This topic is where requests for deactivating current processes are sent. Any message recieved on this topic will trigger the deactivation regardless of payload. Upon reception, the device will close all valves, end all ongoing dispensation or drain processes, and send summary reports of any ongoing processes.

Inputs: None

Sample payload:
```
{}
```

### Restart

| Topic string config  | Publish or subscribe? | Enabled unconditionally? |
| ------------- | ------------- | ------------- |
| `RESTART_TOPIC_`  | Subscribe | Yes |

This topic is where requests for restarting the controller are sent. Any message recieved on this topic will trigger the restart regardless of payload. Upon reception, the device will power down and restart.

Inputs: None

Sample payload:
```
{}
```

### Logging

These topics are where the device will send status logs alongside events and changes in state.

#### Info

| Topic string config  | Publish or subscribe? | Enabled unconditionally? |
| ------------- | ------------- | ------------- |
| `LOG_TOPIC_`  | Subscribe | Yes |

This topic is where the device sends status logs that aren't warnings or errors. Once connected to the MQTT broker, most status changes are reflected here. 

Outputs:
- `m` string. The info log message.

Sample payload:
```
{
  "m": "Beginning dispensation process with target volume: 10.25 liters"
}
```

#### Warnings

| Topic string config  | Publish or subscribe? | Enabled unconditionally? |
| ------------- | ------------- | ------------- |
| `WARNING_TOPIC_`  | Subscribe | Yes |

This topic is where the device sends status logs about events that don't break things but may result in abnormal function.

Outputs:
- `m` string. The warning log message.

Sample payload:
```
{
  "m": "Flow rate exceeded maximum"
}
```

#### Errors

| Topic string config  | Publish or subscribe? | Enabled unconditionally? |
| ------------- | ------------- | ------------- |
| `ERROR_TOPIC_`  | Subscribe | Yes |

This topic is where the device sends status logs about events that prevent sucessful function.

Outputs:
- `m` string. The error log message.

Sample payload:
```
{
  "m": "Dispense request denied, process already in progress."
}
```

### Config

#### Read

#### Write

### Reset Settings

### Drain

#### Activation

#### Reporting

# Hardware

## Operating Modes

## Recommended Equipment

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
    - `true` A flow sensor measures the flow rate. This is useful if the source has a variable flow, or if a tank (and thus flow sensor) is already being used.
    - `false` A flow sensor is not being used. A static flow rate for the source must be configured through `STATIC_FLOW_RATE_DEFAULT`. 

-  `USING_DRAIN_VALVE bool`
    - `true`
    - `false`

- `USING_PRESSURE_SENSOR bool`
    - `true`
    - `false`

### WiFi Manager

These settings are used to configure the WiFi Manager, an instance of the WiFiManager class (see [dependencies](#dependencies)). This class enables the configuration of the WiFi network and MQTT broker through an access point hosted by the controller, rather than being hard coded into the software.

- `AP_NAME string`
- `AP_PASSWORD string`
- `AP_RETRY_DELAY int`
- `AP_IP IPAddress()`
- `AP_GATEWAY IPAddress()`
- `AP_SUBNET IPAddress()`

### MQTT

These settings are used to configue the MQTT client, an instance of the PubSubClient class(see [dependencies](#dependencies)). This class enables interaction with an MQTT broker. The settings marked as `DEFAULT` are able to be configured through the WiFi Manager access point.

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

You can leave these unchanged with no problem, but you might want to customize them. If you do, just keep in mind that the topic strings are sent with every message, and that the MQTT client has a maximum message size of 256 bytes (including header). Try to keep them small, especially for topics with a high message frequency (like `DISPENSE_REPORT_SLICE_TOPIC_`), or a large payload (like 'CONFIG_TOPIC'). 

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

### Explanation of future plans for interface through verdantech web


# Bug reporting and contributing

Required libraries: 
WiFi manager: https://github.com/tzapu/WiFiManager Version: 2.0.15-rc.1
