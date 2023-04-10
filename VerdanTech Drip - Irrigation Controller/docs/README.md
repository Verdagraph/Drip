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

6. Install the Arduino platform, required libraries, and board drivers

7. Configure the device through the file `config.h`

8. Flash the microcontroller with the software

9. Connect to a WiFi network and MQTT broker through the auto-connect access point

10. Calibrate flow sensor

## Terminology

Refer to this list for explicit descriptions of some key terms used throughout the document.

### Water Supplies

- **Source**
    - A supply of water that is assumed to be inexhaustible with a static or variable flow rate, such as a utility water supply.
- **Tank**
    - A supply of water that is assumed to be exhaustible with a variable flow rate, such as a rain barrel.

### Equipment

For specific product examples see the [equipment](#equipment) section.

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

### Operation

- **Dispensation**
    - A process of attempting to output a target volume of water, first from the tank, then from the source.
- **Drain**
    - A process of attempting to release water from the tank without it going to the usual intended output, to prevent overflow, or to get rid of stale water.

## MQTT interface

This section describes the functionality of the MQTT topics through which the device sends and recieves information. All communication is done via JSON. For topics which are subscribed to by the controller, a list of inputs is described for each topic, and for topics which are published by the controller, a list of outputs are described for each topic. Inputs are marked (optional) if the  function will still be carried out if they aren't included rather than throw an error. Inputs are marked (conditional) if they only have effect based on [configuration settings](#configuration), and outputs are marked (conditional) if their inclusion in the payload is dependent on [configuration settings](#configuration).

| Title   | Topic string config  | Does the controller publish or subscribe? | Is the topic enabled unconditionally? |
| ------------- | ------------- | ------------- | ------------- |
| [Dispense Activate](#activation)  | [`DISPENSE_ACTIVATE_TOPIC_`](#auto-config) | Subscribe | Yes |
| [Dispense Slice Report](#slice-reporting)  | [`DISPENSE_REPORT_SLICE_TOPIC_`](#auto-config)  | Publish | Yes |
| [Dispense Summary Report](#summary-reporting)  | [`DISPENSE_REPORT_SUMMARY_TOPIC_`](#auto-config)  | Publish | Yes |
| [Deactivate](#deactivation)  | [`DEACTIVATE_TOPIC_`](#auto-config)  | Subscribe | Yes |
| [Restart](#restart)  | [`RESTART_TOPIC_`](#auto-config)  | Subscribe | Yes |
| [Info Logging](#info)  | [`LOG_TOPIC_`](#auto-config)  | Publish | Yes |
| [Warning Logging](#warnings)  | [`WARNING_TOPIC_`](#auto-config)  | Publish | Yes |
| [Error Logging](#errors)  | [`ERROR_TOPIC_`](#auto-config)  | Publish | Yes |
| [Read Config](#read)  | [`CONFIG_TOPIC_`](#auto-config)  | Publish | Yes |
| [Write Config](#write)  | [`CONFIG_CHANGE_TOPIC_`](#auto-config)  | Subscribe | Yes |
| [Reset Settings](#reset-settings)  | [`SETTINGS_RESET_TOPIC_`](#auto-config)  | Subscribe | Yes |
| [Drain Activate](#activation-1)  | [`DRAIN_ACTIVATE_TOPIC_`](#auto-config)  | Subscribe | No |
| [Drain Summary Report](#reporting)  | [`DRAIN_REPORT_SUMMARY_TOPIC_`](#auto-config)  | Publish | No |

### Dispensation

The main functionality of the device. The dispentation topics handle setting target volumes for the dispensation process, publishing data taken at regular intervals through the process, and publishing data at the end of the process.

#### Activation

| Topic string config  | Publish or subscribe? | Enabled unconditionally? |
| ------------- | ------------- | ------------- |
| [`DISPENSE_ACTIVATE_TOPIC_`](#auto-config)  | Subscribe | Yes |

This topic allows activating the dispensation process and setting the target volume. Activation will fail if a dispensation process or drain process is already in progress. The dispensation process will continue until the output volume is greater than or equal to the target volume, or a singularly connected tank produces a flow rate less than that defined by [`FlowSensorConfig.min_flow_rate`](#runtime-config) and the current duration of the process is greater than that defined by [`TankConfig.tank_timeout`](#runtime-config). If the tank is connected with a source, then the dispensation will switch to the source until the target volume is reached.

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
| [`DISPENSE_REPORT_SLICE_TOPIC_`](#auto-config)  | Publish | Yes |

This topic is where the mid-process datapoints on the dispensation process are published at a volume interval in liters defined by [`ServicesConfig.data_resolution_l`](#runtime-config). If the connection to the MQTT broker fails, these reports will fail to be sent and are lost, but the dispensation process will continue. Unfortunately, the [MQTT client](#dependencies) class is only able to send messages at a quality-of-service of 0. 

Outputs:
- `["t"]` float. Current duration of the dispensation process in seconds.
- `["v"]` float. Current output volume of the dispensation process in liters.
- `["q"]` float. The average flow rate of output volume calculated either from the last slice report to the current one (or from the beginning of the process if on the first report) in liters per minute
- `["tp"]` (conditional) float. The average tank gauge pressure calculated from the last slice report to the current one (or from the beginning of the process if on the first report) in hectopascals. Enabled if [`USING_PRESSURE_SENSOR_`](#auto-config) and [`PressureSensorConfig.report_mode`](#runtime-config) equals 1 or 3.
- `["tv"]` (conditional) float. The average tank volume calculated from the last slice report to the current one (or from the beginning of the process if on the first report) in liters. Enabled if [`USING_PRESSURE_SENSOR_`](#auto-config) and [`PressureSensorConfig.report_mode`](#runtime-config) equals 2 or 3. Ensure that the [tank geometry](#runtime-config) is properly configured.

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
| [`DISPENSE_REPORT_SUMMARY_TOPIC_`](#auto-config)  | Publish | Yes |

This topic is where the post-process datapoints on the dispensation process are published. If the connection to the MQTT broker fails, the connection is re-tried once before the message is lost.  Unfortunately, the [MQTT client](#dependencies) class is only able to send messages at a quality-of-service of 0.

Outputs:
- `["tt"]` float. Total duration of the dispensation process in seconds.
- `["vt"]` float. Total output volume of the dispensation process in liters.
- `["tv"]` (conditional) float. The total output volume produced by the tank in liters. Enabled if [`USING_TANK_`](#auto-config).
- `["tts"]` (conditional) int. The time stamp at which the dispensation process switched from the tank to the source. Will equal `tt` if the processes ended without switching water supplies. Enabled if [`USING_TANK_`](#auto-config) and [`USING_SOURCE_`](#auto-config).

Sample payload:
```
{
  "tt": 50,
  "vt": 10.25,
  "tv": 5.6,
  "tts": 250000,
}
```

### Deactivation

| Topic string config  | Publish or subscribe? | Enabled unconditionally? |
| ------------- | ------------- | ------------- |
| [`DEACTIVATE_TOPIC_`](#auto-config)  | Subscribe | Yes |

This topic is where requests for deactivating current processes are sent. Any message recieved on this topic will trigger the deactivation regardless of payload. Upon reception, the device will close all valves, end all ongoing dispensation or drain processes, and send summary reports of any ongoing processes.

Inputs: None

Sample payload:
```
{}
```

### Restart

| Topic string config  | Publish or subscribe? | Enabled unconditionally? |
| ------------- | ------------- | ------------- |
| [`RESTART_TOPIC_`](#auto-config)  | Subscribe | Yes |

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
| [`LOG_TOPIC_`](#auto-config)  | Publish | Yes |

This topic is where the device sends status logs that aren't warnings or errors. Once connected to the MQTT broker, most status changes are reflected here. 

Outputs:
- `["m"]` string. The info log message.

Sample payload:
```
{
  "m": "Beginning dispensation process with target volume: 10.25 liters"
}
```

#### Warnings

| Topic string config  | Publish or subscribe? | Enabled unconditionally? |
| ------------- | ------------- | ------------- |
| [`WARNING_TOPIC_`](#auto-config)  | Publish | Yes |

This topic is where the device sends status logs about events that don't break things but may result in abnormal function.

Outputs:
- `["m"]` string. The warning log message.

Sample payload:
```
{
  "m": "Flow rate exceeded maximum"
}
```

#### Errors

| Topic string config  | Publish or subscribe? | Enabled unconditionally? |
| ------------- | ------------- | ------------- |
| [`ERROR_TOPIC_`](#auto-config)  | Publish | Yes |

This topic is where the device sends status logs about events that prevent sucessful function.

Outputs:
- `["m"]` string. The error log message.

Sample payload:
```
{
  "m": "Dispense request denied, process already in progress."
}
```

### Config

These topics allow viewing and updating configuration settings at runtime. The available settings are listed as [config defaults](#defaults). 

#### Read

| Topic string config  | Publish or subscribe? | Enabled unconditionally? |
| ------------- | ------------- | ------------- |
| [`CONFIG_TOPIC_`](#auto-config)  | Publish | Yes |

This topic is where the device will send the current configuration values. It sends a retained message containing all values on startup and every time the configuration is changed.

Outputs:
- `["srvc"]["res"]` float. The value of [`ServicesConfig.data_resolution_l`](#runtime-config).
- `["src"]["rate"]` (conditional) float. The value of [`SourceConfig.static_flow_rate`](#runtime-config). Enabled if [`USING_SOURCE_`](#auto-config).
- `["tnk"]["time"]` (conditional) int. The value of [`TankConfig.tank_timeout`](#runtime-config). Enabled if [`USING_TANK_`](#auto-config).
- `["tnk"]["shape"]` (conditional) int. The value of [`TankConfig.shape_type`](#runtime-config). Enabled if [`USING_TANK_`](#auto-config).
- `["tnk"]["dim1"]` (conditional) float. The value of [`TankConfig.dimension_1`](#runtime-config). Enabled if [`USING_TANK_`](#auto-config).
- `["tnk"]["dim2"]` (conditional) float. The value of [`TankConfig.dimension_2`](#runtime-config). Enabled if [`USING_TANK_`](#auto-config).
- `["tnk"]["dim3"]` (conditional) float. The value of [`TankConfig.dimension_3`](#runtime-config). Enabled if [`USING_TANK_`](#auto-config).
- `["flow"]["ppl"]` (conditional) float. The value of [`FlowSensorConfig.pulses_per_l`](#runtime-config). Enabled if [`USING_FLOW_SENSOR_`](#auto-config).
- `["flow"]["max"]` (conditional) float. The value of [`FlowSensorConfig.max_flow_rate`](#runtime-config). Enabled if [`USING_FLOW_SENSOR_`](#auto-config).
- `["flow"]["min"]` (conditional) float. The value of [`FlowSensorConfig.min_flow_rate`](#runtime-config). Enabled if [`USING_FLOW_SENSOR_`](#auto-config).
- `["prssr"]["mode"]` (conditional) int. The value of [`PressureSensorConfig.report_mode`](#runtime-config). Enabled if [`USING_PRESSURE_SENSOR_`](#auto-config).
- `["prssr"]["atmo"]` (conditional) float. The value of [`PressureSensorConfig.atmosphere_pressure`](#runtime-config). Enabled if [`USING_PRESSURE_SENSOR_`](#auto-config).

Sample payload:
```
```

#### Write

| Topic string config  | Publish or subscribe? | Enabled unconditionally? |
| ------------- | ------------- | ------------- |
| [`CONFIG_CHANGE_TOPIC_`](#auto-config)  | Subscribe | Yes |

This topic is where the device will accept changes to the configuration values. All the inputs are optional, meaning any combination of inputs will be accepted, and only the settings that are to be changed must be sent. Upon reception of the message, the device will update the [config structs](#runtime-config), write the new values to the file system, and publish the new values to [`CONFIG_TOPIC_`](#auto-config).

Inputs:
- `["srvc"]["res"]` (optional) float. The value of [`ServicesConfig.data_resolution_l`](#runtime-config) to be set.
- `["src"]["rate"]` (optional, conditional) float. The value of [`SourceConfig.static_flow_rate`](#runtime-config) to be set.
- `["tnk"]["time"]` (optional, conditional) int. The value of [`TankConfig.tank_timeout`](#runtime-config) to be set.
- `["tnk"]["shape"]` (optional, conditional) int. The value of [`TankConfig.shape_type`](#runtime-config) to be set.
- `["tnk"]["dim1"]` (optional, conditional) float. The value of [`TankConfig.dimension_1`](#runtime-config) to be set.
- `["tnk"]["dim2"]` (optional, conditional) float. The value of [`TankConfig.dimension_2`](#runtime-config) to be set.
- `["tnk"]["dim3"]` (optional, conditional) float. The value of [`TankConfig.dimension_3`](#runtime-config) to be set.
- `["flow"]["ppl"]` (optional, conditional) float. The value of [`FlowSensorConfig.pulses_per_l`](#runtime-config) to be set.
- `["flow"]["max"]` (optional, conditional) float. The value of [`FlowSensorConfig.max_flow_rate`](#runtime-config) to be set.
- `["flow"]["min"]` (optional, conditional) float. The value of [`FlowSensorConfig.min_flow_rate`](#runtime-config) to be set.
- `["prssr"]["mode"]` (optional, conditional) int. The value of [`PressureSensorConfig.use_calibration`](#runtime-config) to be set.
- `["prssr"]["atmo"]` (optional, conditional) float. The value of [`PressureSensorConfig.atmosphere_pressure`](#runtime-config) to be set.

Sample payload:
```
```

### Reset Settings

| Topic string config  | Publish or subscribe? | Enabled unconditionally? |
| ------------- | ------------- | ------------- | 
| [`SETTINGS_RESET_TOPIC_`](#auto-config)  | Subscribe | Yes |

This topic can be used to reset settings to their default values.

Inputs:
- `["wifi"]` (optional) bool. If `true`, run the function [`WiFiManager.resetSettings()`](#dependencies) and restart the ESP. The default auto-connect access point will return.
- `["mqtt"]` (optional) bool. If `true`, delete the MQTT config file and restart the ESP. The default [MQTT settings](#mqtt) will be used, and if the connection isn't successfull, the auto-connect access point will return.

Sample payload:
```
```

### Drain

The drain topics handle setting target times, pressures, or volumes for the drain process, publishing data at the end of the process. Enabled if `USING_DRAIN_VALVE_`. 

#### Activation

| Topic string config  | Publish or subscribe? | Enabled unconditionally? |
| ------------- | ------------- | ------------- |
| [`DRAIN_ACTIVATE_TOPIC_`](#auto-config)  | Subscribe | No |

This topic allows activating the drain process and setting either the target time, pressure, or volume. Activation will fail if a drain process or dispensation process is already in progress. While inputs are listed as optional, the process will only start if exactly one target is sent. The drain process will continue until the duration of the process is greater than or equal to the target time, the tank gauge pressure is less than or equal to the target pressure, or the tank volume is less than or equal to the target volume, depending on which target was sent.

Inputs:
- `["tt"]` (optional) int. Target time of the drain process in seconds.
- `["tp"]` (optional, conditional) float. Target pressure of the drain process in hectopascals. Enabled if `USING_PRESSURE_SENSOR_`.
- `["tv"]` (optional, conditional) float. Target volume of the drain process in liters. Enabled if `USING_PRESSURE_SENSOR_`. Ensure that the [tank geometry](#runtime-config) is properly configured.

Sample payload:
```
```

#### Summary Reporting

| Topic string config  | Publish or subscribe? | Enabled unconditionally? |
| ------------- | ------------- | ------------- |
| [`DRAIN_REPORT_SUMMARY_TOPIC_`](#auto-config)  | Publish | No |

This topic is where the post-process datapoints on the drain process are published. If the connection to the MQTT broker fails, the connection is re-tried once before the message is lost. Unfortunately, the [MQTT client](#dependencies) class is only able to send messages at a quality-of-service of 0.

Outputs:
- `["tt"]` float. Total duration of the drain process in seconds.
- `["sp"]` (conditional) float. The inital tank gauge pressure of the drain process in hectopascals. Enabled if [`USING_PRESSURE_SENSOR_`](#auto-config) and [`PressureSensorConfig.report_mode`](#runtime-config) equals 1 or 3.
- `["fp"]` (conditional) float. The final tank gauge pressure of the drain process in hectopascals. Enabled if [`USING_PRESSURE_SENSOR_`](#auto-config) and [`PressureSensorConfig.report_mode`](#runtime-config) equals 1 or 3.
- `["sv"]` (conditional) float. The inital tank volume of the drain process in liters. Enabled if [`USING_PRESSURE_SENSOR_`](#auto-config) and [`PressureSensorConfig.report_mode`](#runtime-config) equals 2 or 3. Ensure that the [tank geometry](#runtime-config) is properly configured.
- `["fv"]` (conditional) float. The final tank volume of the drain process in liters. Enabled if [`USING_PRESSURE_SENSOR_`](#auto-config) and [`PressureSensorConfig.report_mode`](#runtime-config) equals 2 or 3. Ensure that the [tank geometry](#runtime-config) is properly configured.

Sample payload:
```
```

# Hardware

## Operating Modes

## Equipment

In this section, I'll list all the equipment I used for this project. This isn't an exhaustive list of all parts that could be used: it's just a list of parts I used, and I build a controller with a source, tank, flow sensor, pressure sensor, and drain. The substitution of some parts, like the type of tubes and fittings, are simple, but other parts, like the microcontroller and sensors, will require significant modifications to the software.

Several of the items are 3D printed. You can find the 3D mesh (.stl) and the Fusion360 (.f3d) files in the CAD folder of the project directory. Unless you're using the exact same products as I am, the 3D models likely won't work for you, and you may find them inadequate in other aspects. Unfortunately, at the time I created these models, I didn't have knowledge of best practices for Fusion360, and so you may have a hard time modifying them for your purposes. Hopefully for those who start from scratch, my models will help, and please let me know if you create any so we can add them to the repo.

Each section will have a table in this format for summary:

| Item | Quantity | Where to source | Product Link |
| -------------  | ------------- | ------------- | ------------- |
| Name of the item  | Will vary depending on other equipment and the scale of the irrigation network | Where I sourced it | The exact product I used |

### Core

These items form the core functionality of the device.

| Item | Quantity | Where to source | Product Link |
| -------------  | ------------- | ------------- | ------------- |
| [Microcontroller](#microcontroller)  | 1 | Hobby shop | [Adafruit](https://www.adafruit.com/product/2471) |
| [Solenoid valve](#solenoid-valve) | 1 | Hobby shop | [BC Robotics](https://bc-robotics.com/shop/12v-solenoid-valve-34/) |
| [Solenoid valve (no minimum pressure)](#solenoid-valve-no-minimum-pressure) | 0-2 | Hobby shop | [BC Robotics](https://bc-robotics.com/shop/12v-solenoid-valve-no-minimum-pressure-34/) |
| [Flow sensor](#flow-sensor)  | 0-1 | Hobby shop | [Digiten](https://www.digiten.shop/products/digiten-g3-8-water-flow-hall-sensor-switch-flow-meter-0-3-10l-min) |
| [Pressure sensor](#pressure-sensor) | 0-1 | Hobby shop | [Adafruit](https://www.adafruit.com/product/3965) |

#### Microcontroller

The brains of the controller. **Required for every project.** I used the ESP8266 because of its price, availability, ease of programming, and most importantly: built-in WiFi. If you're using a different microcontroller, you will need to modify the software. The controller has some pin requirements you'll need to keep in mind if you're not using the Adafruit ESP I used:

- Enough pins for each of the valve(s).
- Interrupt capable pin (if [`USING_FLOW_SENSOR_`](#auto-config)).
- I2C pins (if [`USING_PRESSURE_SENSOR`](#auto-config)).
- Deep sleep capability (if using [`AP_RETRY_DEEP_SLEEP`](#wifi-manager)).

#### Solenoid Valve

The main mechanical function of the device. **Required if [`USING_SOURCE_`](#auto-config).** These valves, which have a minimum pressure requirement to open/close, are suitable for use with a source, but not a tank, because a source is assumed to have a high enough pressure at all times. I used a 12V solenoid valve because I had a 12V power supply and it seemed a suitable choice. Keep in mind that for every valve you will need a [diode](#diode) to prevent EMF feedback, and that you will need a [relay](#relay) with a rated voltage that matches the valve's voltage, with at least as many modules as valves. 

#### Solenoid Valve (No Minimum Pressure)

The main mechanical function of the device. **Required if [`USING_TANK_`](#auto-config).** These valves, which do not have a minimum pressure requirement to open/close, are suitable for use with a tank and a source, because a tank is assumed to possibly have zero pressure. I used a 12V solenoid valve because I had a 12V power supply and it seemed a suitable choice. Keep in mind that for every valve you will need a [diode](#diode) to prevent EMF feedback, and that you will need a [relay](#relay) with a rated voltage that matches the valve's voltage, with at least as many modules as valves. 

#### Relay

Used for switching the high voltage valves with the low voltage of the microcontroller. **Required for every project**. You will need a relay with a rated voltage that matches the valve's voltage, with at least as many modules as valves. The product I linked is a 4-module relay, because I'm using 3 valves and I couldn't find a 3-module relay. Make sure to test that your relay modules work, as I've had some bad luck in the past with recieving dead relays.

#### Flow Sensor

Used for sensing the rate of water flow through the tank's output, or the tank and source's output. **Required if [`USING_TANK_`](#auto-config) or if [`USING_SOURCE_`](#auto-config) and [`USING_SOURCE_FLOW`](#core-1).** The flow sensor I used is a hall effect flow sensor, which uses a magnetic field to send voltages pulses into a data wire. 
I used the sensor I did because the flow rate range (0.6-10L/min) seemed like it would capture what would be coming out of my water supplies, and because the 3/8 inch thread was convienent to make [adapters](#38-thread-to-34-thread) for.

The sensor uses an interrupt pin to update the controller's memory, specifically a `volatile int.`

Make sure to configure [`MAX_FLOW_RATE_DEFAULT`](#defaults) and [`MIN_FLOW_RATE_DEFAULT`](#defaults) for the sensor you choose.

#### Pressure Sensor

### Cases

These items are protective cases around the other items. Most are optional.

| Item | Quantity | Where to source | Product Link |
| -------------  | ------------- | ------------- | ------------- |
| [Controller case](#controller-case) | 1 | 3D Print | CAD folder |
| [Valve case](#valve-case) | 0-3 | 3D Print | CAD folder |
| [Flow sensor case](#flow-sensor-case) | 0-1 | 3D Print | CAD folder |
| [pressure sensor case](#pressure-sensor-case) | 0-1 | 3D Print | CAD folder |

#### Controller Case
#### Valve Case
#### Flow Sensor Case
#### Pressure Sensor Case

### Electrical

These items are electrical components needed to build the control circuit.

This section assumes you already has a soldering setup, solder, and wires

| Item | Quantity | Where to source | Product Link |
| -------------  | ------------- | ------------- | ------------- |
| [Relay](#relay) | 1 | Hobby shop | [BC Robotics](https://bc-robotics.com/shop/4-channel-relay-breakout-12v/) |
| [Power supply](#power-supply) | 1 | Hobby shop | [BC Robotics](https://bc-robotics.com/shop/12v-3a-power-supply-2-5mm/) |
| [Power supply adapter](#power-supply-adapter) | 1 | Hobby shop | [BC Robotics](https://bc-robotics.com/shop/dc-barrel-jack-to-2-pin-terminal-block-adapter/) |
| [Voltage regulator](#voltage-regulator) | 1 | Hobby shop | [RobotShop](https://ca.robotshop.com/products/5v-25a-step-down-voltage-regulator-d24v25f5) |
| [Diode](#diode) | 1-3 | Hobby shop | [BC Robotics](https://bc-robotics.com/shop/diode-rectifier-1a-50v/) |
| [Logic level converter](#logic-level-converter) | 0-1 | Hobby shop | [BC Robotics](https://bc-robotics.com/shop/4-channel-logic-level-converter-bi-directional/) |
| [Board](#board) | 1 | Hobby shop | [BC Robotics](https://bc-robotics.com/?product_cat=&s=proto+board&post_type=product) |
| [2-wire waterproof connector](#2-wire-waterproof-connector) | 0-3 | Hobby shop | [BC Robotics](https://bc-robotics.com/shop/waterproof-dc-power-cable-set/) |
| [4-wire waterproof connector](#4-wire-waterproof-connector) | 0-2 | Hobby shop | [BC Robotics](https://bc-robotics.com/shop/waterproof-polarized-4-wire-cable-set/) |

#### Relay
#### Power Supply
#### Power Supply Adapter
#### Voltage Regulator
#### Diode
#### Logic Level Converter
#### Board
#### 2-Wire Waterproof Connector
#### 4-Wire Waterproof Connector

### Fluids

These items are needed to handle the water from the water supplies.

| Item | Quantity | Where to source | Product Link |
| -------------  | ------------- | ------------- | ------------- |
| [1/2 poly](#12-poly) | 1+ | Irrigation supplier | [Irrigation Direct](https://www.irrigationdirect.ca/DD-DH700-100-1-2-x-100-.700-OD-Poly-Tubing-100-Roll-DD-DH700-100) |
| [1/4 poly](#14-poly) | 1+ | Irrigation supplier | [Irrigation Direct](https://www.irrigationdirect.ca/DD-DH250-100-1-4-x-100-Poly-Micro-Tubing-100-Roll-DD-DH250-100) |
| [Pressure regulator](#pressure-regulator) | 0-1 | Irrigation supplier | [Irrigation Direct](https://www.irrigationdirect.ca/dd-hpr10-drip-irrigation-pressure-regulator-10-psi-hose-threaded-3-4-c2c1-fht-x-3-4-mht.) |
| [Backflow preventer](#backflow-preventer) | 0-1 | Irrigation supplier | [Irrigation Direct](https://www.irrigationdirect.ca/dd-hvb-vacuum-breaker-3-4-hose-thread-anti-siphon-backflow-preventer-for-drip-irrigation.-c2c76) |
| [Vacuum relief valve](#vacuum-relief-valve) | 1 | Irrigation supplier | [Irrigation Direct](https://www.irrigationdirect.ca/dt-arv75-3-4-mpt-air-vacuum-relief-valve-c2f2) |
| [Filter](#filter) | 1 | Irrigation supplier | [Irrigation Direct](https://www.irrigationdirect.ca/Irritec-A4-HTF155-Drip-Irrigation-Y-Filter-3-4-Female-Hose-Swivel-x-Male-Hose-150-Stainless-Steel-Mesh-Screen) |

#### 1/2 Poly
#### 1/4 Poly
#### Pressure Regulator
#### Backflow Preventer
#### Vacuum Relief Valve
#### Filter

### Fittings

These items are needed to fit together the fluids supplies.

| Item | Quantity | Where to source | Product Link |
| -------------  | ------------- | ------------- | ------------- |
| [1/2 poly to 3/4 MHT](#12-poly-to-34-mht) | 1+ | Irrigation supplier | [Irrigation Direct](https://www.irrigationdirect.ca/dl-mh600-drip-irrigation-direct-loc-male-hose-end-.600-id-el-x-3-4-mht-700-or-710-tubing-b1b5) |
| [1/2 poly to 3/4 FHT](#12-poly-to-34-fht) | 1+ | Irrigation supplier | [Irrigation Direct](https://www.irrigationdirect.ca/DL-FHS600-Direct-Loc-Swivel-Adapter-.600-ID-DL-x-3-4-FHT-Swivel-700-or-710-Tubing.-DL-FHS600) |
| [1/2 poly to poly tee join](#12-poly-tee-join) | 0+ | Irrigation supplier | [Irrigation Direct](https://www.irrigationdirect.ca/DL-T600-Direct-Loc-Tee-.600-ID-700-or-710-Tubing.-DL-T600) |
| [1/2 poly elbow join](#12-poly-elbow-join) | 0+ | Irrigation supplier | [Irrigation Direct](https://www.irrigationdirect.ca/DL-L600-Direct-Loc-Elbow-.600-ID-DL-x-DL-700-or-710-Tubing.-DL-L600) |
| [Tank bulkhead](#tank-bulkhead) | 0-3 | 3D print | CAD folder |
| [Hose washer](#hose-washer) | 0-10 | Hardware store | [Home Depot](https://www.homedepot.ca/product/orbit-hose-washer-combo-12-pack-/1000474848) |
| [Tank bulkhead to 3/4 thread](#tank-bulkhead-to-34-thread) | 0-2 | 3D print | CAD folder |
| [Tank bulkhead to 1/4 poly](#tank-bulkhead-to-14-poly) | 0-1 | 3D print | CAD folder |
| [3/8 thread to 3/4 thread](#38-thread-to-34-thread) | 0-2 | 3D print | CAD folder |

#### 1/2 Poly to 3/4 MHT
#### 1/2 Poly to 3/4 FHT
#### 1/2 Poly Tee Join
#### 1/2 Poly Elbow Join
#### Tank Bulkhead
#### Hose Washer
#### Tank Bulkhead to 3/4 Thread
#### Tank Bulkhead to 1/4 Poly
#### 3/8 Thread to 3/4 Thread


### Irrigation System

These items are what I used to build out the rest of the irrigation system, after the section controlled by the irrigation controller. These items are going to be highly variable on the space the irrigation system is covering, and can be considered seperate from the system of the controller. I've just listed the items I used for reference.  

| Item | Quantity | Where to source | Product Link |
| -------------  | ------------- | ------------- | ------------- |
| [Inline valve](#inline-valve) | 0+ | Irrigation supplier | [Irrigation Direct](https://www.irrigationdirect.ca/DL-FCV600-Pema-Loc-1-2-Flow-Control-Valve-DL-x-DL.-DL-FCV600) |
| [Tube cutter](#tube-cutter) | 1 | Irrigation supplier | [Irrigation Direct](https://www.irrigationdirect.ca/tl-tc100-3/4-tubing-cutter-b2e8) |
| [1/2 poly end](#12-poly-end) | 1+ | Irrigation supplier | [Irrigation Direct](https://www.irrigationdirect.ca/dd-f8-figure-8-end-of-line-plug.-b1c6) |
| [1/2 poly mount](#12-poly-mount) | 1+ | Irrigation supplier | [Irrigation Direct](https://www.irrigationdirect.ca/dd-mc700b-1-2-tubing-mounting-clamp-with-nail-black.-b2d2) |
| [Drip tape](#drip-tape) | 1+ | Irrigation supplier | [Irrigation Direct](https://www.irrigationdirect.ca/p1-1508-500-irritec-p1-drip-tape-15mil-x-8-spacing-.25-gpm-100ft) |
| [Drip tape barb](#drip-tape-barb) | 1+ | Irrigation supplier | [Irrigation Direct](https://www.irrigationdirect.ca/DT-TO250-Drip-Tape-5-8-Loc-x-1-4-Barb-DT-TO250) |
| [Drip tape hole punch](#drip-tape-hole-punch) | 1 | Irrigation supplier | [Irrigation Direct](https://www.irrigationdirect.ca/dd-hp250-dg-hole-punch-for-1-4-barbed-fittings-and-drippers-b2e3) |
| [Drip tape sleeve end](#drip-tape-sleeve-end) | 1+ | Irrigation supplier | [Irrigation Direct](https://www.irrigationdirect.ca/DT-TSE-Drip-Tape-5-8-Sleeve-End-DT-TS) |
| [Drip tape stake](#drip-tape-stake) | 1+ | Irrigation supplier | [Irrigation Direct](https://www.irrigationdirect.ca/TLS6-Netafim-Drip-Tubing-Hold-Down-6-Wire-Stake-Staple-Style-fits-1-8-to-3-4-tubing-DD-S6.html) |

#### Inline Valve
#### Tube Cutter
#### 1/2 Poly End
#### 1/2 Poly mount
#### Drip Tape
#### Drip Tape Barb
#### Drip Tape Hole Punch
#### Drip Tape Sleeve End
#### Drip Tape Stake


## Assembly

### Water Supplies

### Electrical

deep sleep

### Fluids

# Software configuration

## Configuration

All configuration settings can be found in the file `config.h`. **Assume that all settings require your attention unless otherwise stated.**

### Core

These settings define the [operating mode](#operating-modes) of the device. Make sure you have the correct hardware connected before you enable any related settings here.

- `DEBUG bool` Defines whether to output debugging info to the serial log.

- `RESEVOIR_MODE int` Defines which [types of water supplies](#water-supplies) the controller is connected to.
    - `1` Source
    - `2` Tank
    - `3` Both

- `USING_SOURCE_FLOW bool` If using a source, define the method of flow rate estimation. 
    - `true` A flow sensor measures the flow rate. This is useful if the source has a variable flow, or if a tank (and thus flow sensor) is already being used.
    - `false` A flow sensor is not being used. A static flow rate for the source must be configured through [`SourceConfig.static_flow_rate`](#runtime-config). 

-  `USING_DRAIN_VALVE bool` If using a tank, define whether the controller is connected to a drain valve on the tank.
    - `true`
    - `false`

- `USING_PRESSURE_SENSOR bool` If using a tank, define whether the controller is connected to a pressure sensor measuring the absolute pressure at the bottom of the tank.
    - `true`
    - `false`

### WiFi Manager

These settings are used to configure the WiFi Manager, an instance of the WiFiManager class (see [dependencies](#dependencies)). This class enables the configuration of the WiFi network and MQTT broker through an access point hosted by the controller, rather than being hard coded into the software.

- `AP_NAME string` The SSID of the auto-connect access point.

- `AP_PASSWORD string` The password of the auto-connect access point.

- `AP_IP IPAddress()` The IP address of the auto-connect access point. **You should ensure this doesn't conflict with another device on your network, but otherwise doesn't need to be changed.**

- `AP_GATEWAY IPAddress()` The gateway of the auto-connect access point. **This likely does not need to be changed.**

- `AP_SUBNET IPAddress()` The subnet of the auto-connect access point. **This likely does not need to be changed.**

- `AP_TIMEOUT int` The duration of time the auto-connect access point is kept open in seconds. **This likely does not need to be changed.**

- `AP_RETRY_DELAY int` The duration of time to wait between auto-connect access point timeout and next try in seconds. **This likely does not need to be changed.**

- `AP_RETRY_DEEP_SLEEP bool` Define the method used to wait for `AP_RETRY_DELAY`.
    - `true` The ESP8266 function `ESP.deepSleep()` will be used to reduce power consumption. See the [electrical assembly](#electrical) section for the required electrical wiring.
    - `false` The Arduino function `delay()` followed by `ESP.restart()` will be used.

### MQTT

These settings are used to configue the MQTT client, an instance of the PubSubClient class(see [dependencies](#dependencies)). This class enables interaction with an MQTT broker. The settings marked as `DEFAULT` are able to be configured through the WiFi Manager access point.

- `MQTT_SERVER_DOMAIN_DEFAULT string` The domain, or URL, of the MQTT broker to connect to.

- `MQTT_SERVER_PORT_DEFAULT string` The port on the MQTT broker to connect to.

- `MQTT_ID_DEFAULT string` The ID of the client.

- `MQTT_USERNAME_DEFAULT string` The username of the client.

- `MQTT_PASSWORD_DEFAULT string` The password of the client.

- `MQTT_RETRY_TIMEOUT int` The duration of time to continually attempt connection to the MQTT broker before returning to the auto-connect access point, in seconds. **This likely does not need to be changed.**

### Pins

These settings define the pins used to interact with the equipment connected to the controller. Don't worry about changing the settings that don't apply to your use case - unused pins are automatically set to -1 in the [auto-config](#auto-config). 

- `SOURCE_OUTPUT_VALVE_PIN int` The output pin connected to the source output valve. High signals should open the valve.

- `TANK_OUTPUT_VALVE_PIN int` The output pin connected to the tank output valve. High signals should open the valve.

- `TANK_DRAIN_VALVE_PIN int` The output pin connected to the tank drain valve. High signals should open the valve.

- `FLOW_SENSOR_PIN int` The input pin connected to the flow sensor data wire. See the [equipment](#flow-sensor) section to ensure compatible logic levels and interrupt-capability.

### Defaults

These settings are default settings that can be changed at runtime through the [config change topic](#write), where they are written to a file on the controller's file system memory and stored in [config structs](#runtime-config). Sensible defaults should still be set here to avoid having to manually update each setting.  

- `DATA_RESOLUTION_L_DEFAULT float` The default value of [`ServicesConfig.data_resolution_l`](#runtime-config). **This likely does not need to be changed.**

- `STATIC_FLOW_RATE_DEFAULT float` The default value of [`SourceConfig.static_flow_rate`](#runtime-config).

- `TANK_TIMEOUT_DEFAULT int` The default value of [`TankConfig.tank_timeout`](#runtime-config). **This does not need to be changed.**

- `TANK_SHAPE_DEFAULT int` The default value of [`TankConfig.shape_type`](#runtime-config).

- `TANK_DIMENSION_1_DEFAULT float` The default value of [`TankConfig.dimension_1`](#runtime-config).

- `TANK_DIMENSION_2_DEFAULT float` The default value of [`TankConfig.dimension_2`](#runtime-config).

- `TANK_DIMENSION_3_DEFAULT float` The default value of [`TankConfig.dimension_3`](#runtime-config).

- `PULSES_PER_L_DEFAULT float` The default value of [`FlowSensorConfig.pulses_per_l`](#runtime-config).

- `MAX_FLOW_RATE_DEFAULT float` The default value of [`FlowSensorConfig.max_flow_rate`](#runtime-config).

- `MIN_FLOW_RATE_DEFAULT float` The default value of [`FlowSensorConfig.min_flow_rate`](#runtime-config).

- `PRESSURE_REPORT_MODE_DEFAULT int` The default value of [`PressureSensorConfig.report_mode`](#runtime-config). **This does not need to be changed.**

- `ATMOSPHERIC_PRESSURE_HPA_DEFAULT float` The default value of [`PressureSensorConfig.atmosphere_pressure`](#runtime-config). **This does not need to be changed. Unless you live on Mount Everest.**

### Runtime Config

These settings are initialized at runtime, first reading a configuration file from the file system, then using and writing to the file the [default values](#defaults) if the file doesn't exist. The file can be update at runtime through the [config write](#write) topic. 

- `ServicesConfig.data_resolution_l = float` The volume interval at which slice reports are sent to the [dispense slice report](#slice-reporting) topic during the dispensation process, in liters. 

- `SourceConfig.static_flow_rate = float` The static flow rate to use for the source output volume in liters. Enabled if [`USING_SOURCE_FLOW`](#core) is false.

- `TankConfig.tank_timeout = int` The delay in seconds between the start of a dispense process and the first moment a flow rate lower than `FlowSensorConfig.min_flow_rate` can result in either the dispense process ending or switching to the source, depending on the value of [`USING_SOURCE_`](#auto-config).

- `TankConfig.shape_type = int` The geometric shape of the tank, used in the conversion from tank pressure to volume. This conversion is done with a function called `pressure_to_volume()` in the file `app.cpp`.
    - `1` The tank is a rectangular prism. `TankConfig.dimension_1` describes length, `TankConfig.dimension_2` describes width, and `TankConfig.dimension_3` describes height.
    - `2` The tank is a cylinder. `TankConfig.dimension_1` describes diameter, `TankConfig.dimension_2` describes height, and `TankConfig.dimension_3` is unused.

- `TankConfig.dimension_1 = float` The first dimension of the tank's geometry, depending on `TankConfig.shape_type`.

- `TankConfig.dimension_2 = float` The second dimension of the tank's geometry, depending on `TankConfig.shape_type`.

- `TankConfig.dimension_3 = float` The third dimension of the tank's geometry, depending on `TankConfig.shape_type`.

- `FlowSensorConfig.pulses_per_l = float` Defines the amount of pulses returned by the flow sensor per liter.

- `FlowSensorConfig.max_flow_rate = float` Defines the maximum rated flow rate of the flow sensor in liters per minute. When the dispensation flow rate surpasses this value, a warning will be sent to [`WARNING_TOPIC_`](#auto-config).

- `FlowSensorConfig.min_flow_rate = float` Defines the minimum rated flow rate of the flow sensor in liters per minute. When the dispensation flow rate goes below this value, a warning will be sent to [`WARNING_TOPIC_`](#auto-config). If the active water supply is the tank, a flow rate below this value, combined with a current dispense duration greater than `TankConfig.tank_timeout`, will result in the dispensation either shutting off or switching to the source, depending on [`USING_SOURCE_`](#auto-config).

- `PressureSensorConfig.report_mode = int` When reporting values related the pressure in the tank, define which values to report.
    - `1` Pressure in hectopascals.
    - `2` Volume in liters. Requires the tank geometry is correctly configured.
    - `3` Both.

- `PressureSensorConfig.atmosphere_pressure = float` The pressure in hectopascals to be subtracted from the absolute pressure returned by the pressure sensor, in order to find the gauge pressure in the tank.

### Topics

These settings define the topic strings to use for the MQTT interface. See the [MQTT interface](#mqtt-interface) for details.

**You can leave these unchanged with no problem,** but you might want to customize them. If you do, just keep in mind that the topic strings are sent with every message, and that the MQTT client has a maximum message size of 256 bytes (including header). Try to keep them small, especially for topics with a high message frequency (like [`DISPENSE_REPORT_SLICE_TOPIC_`](#auto-config)), or a large payload (like [`CONFIG_TOPIC_`](#auto-config)). 

- `BASE_TOPIC string` The base topic used to pre-pend all other topics through the [auto-config](#auto-config).

- `DISPENSE_ACTIVATE_TOPIC string` The topic used to recieve [dispense activation](#activation) requests.

- `DISPENSE_REPORT_SLICE_TOPIC string` The topic used to send [dispense slice reports](#slice-reporting).

- `DISPENSE_REPORT_SUMMARY_TOPIC string` The topic used to send [dispense summary reports](#summary-reporting).

- `DEACTIVATE_TOPIC string` The topic used to recieve [deactivation requests](#deactivation).

- `RESTART_TOPIC string` The topic used to recieve [restart requests](#restart).

- `LOG_TOPIC string` The topic used to send [info logs](#info).

- `WARNING_TOPIC string` The topic used to send [warning logs](#warnings).

- `ERROR_TOPIC string` The topic used to send [error logs](#errors).

- `CONFIG_TOPIC string` The topic used to send current [config values](#read).

- `CONFIG_CHANGE_TOPIC string` The topic used to recieve [config change requests](#write).

- `SETTINGS_RESET_TOPIC string` The topic used to recieve [settings reset requsets](#reset-settings).

- `DRAIN_ACTIVATE_TOPIC string` The topic used to recieve [drain activation](#activation-1) requests.

- `DRAIN_REPORT_SUMMARY_TOPIC string` The topic used to send [drain summary reports](#summary-reporting-1).

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

WiFi manager: https://github.com/tzapu/WiFiManager Version: 2.0.15-rc.1
https://pubsubclient.knolleary.net/ Version: 2.8.0
https://arduinojson.org/ Version: 6.21.0

https://github.com/adafruit/Adafruit_Sensor Version 1.1.8 (For MPRLS library)
https://github.com/adafruit/Adafruit_BusIO Version 1.14.0 (For MPRLS library)
https://github.com/adafruit/Adafruit_MPRLS Version 1.1.0 (For pressure sensor)

## For Contributors

### Explanation of the purposes of different files

### Explanation of app state 

### Explanation of future plans for interface through verdantech web


# Bug reporting and contributing
