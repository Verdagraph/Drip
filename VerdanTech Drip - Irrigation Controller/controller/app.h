// app.h

#ifndef APP_H
#define APP_H

#include "config.h"

namespace app {

// Stores flags on process states
struct FlagStore {
  bool dispense_flag; // True if dispensing
  bool drain_flag; // True if draining
  bool deactivate_flag; // True if pending deactivation
  bool resevoir_switch_flag; // True if dispensing and resevoir has been switched
  bool mqtt_connected_flag; // True if connected to MQTT

  FlagStore() {
    dispense_flag = false;
    drain_flag = false;
    deactivate_flag = false;
    resevoir_switch_flag = false;
    mqtt_connected_flag = true;
  }
};

// Stores timestamps of process states
struct TimeStore {
  unsigned long int process_begin_timestamp; // Initial process timestamp
  unsigned long int last_timestamp; // Last timestamp
  unsigned long int resevoir_switch_timestamp; // Timestamp of resevoir switch

  TimeStore() {
    process_begin_timestamp = 0;
    last_timestamp = 0;
    resevoir_switch_timestamp = 0;
  }
};

// Stores targets for processes
struct TargetStore {
  float target_output_volume; // Target dispensation output in L
  float target_drain_time; // Target drain time in s
  float target_drain_volume; // Target drain volume in L
  float target_drain_pressure; // Target drain pressure in kPa

  TargetStore() {
    target_output_volume = 0;
    target_drain_time = 0;
    target_drain_volume = 0;
    target_drain_pressure = 0;
  }
};

// Stores current and last values for sensors
struct SensorStore {
  volatile int pulses; // Counter of flow sensor pulses
  unsigned int last_pulses; // Previous timestamp pulses
  float pressure; // Current pressure sensor value

  SensorStore() {
    pulses = 0;
    last_pulses = 0;
    pressure = 0;
  }
};

// Stores derived values for current process slice
struct SliceStore {

  unsigned int time_elapsed; // Time since last timestamp
  unsigned int pulses_elapsed; // Pulses since last timestamp
  float output_volume_elapsed; // Volume since last timestamp
  float flow_rate; // Current flow rate
  unsigned int total_time_elapsed; // Time since process start
  float total_output_volume; // Total volume output since process begin

  float current_avg_flow; // Current sum total of flow rate measurments between reports
  int avg_flow_count; // Number of flow rate measurments between reports
  float current_avg_tank_pressure; // Current tank pressure between reports
  int avg_tank_count; // Number of tank pressure measurments between reports

  SliceStore() {
    time_elapsed = 0;
    pulses_elapsed = 0;
    output_volume_elapsed = 0;
    flow_rate = 0;
    total_time_elapsed = 0;
    total_output_volume = 0;
    current_avg_flow = 0;
    avg_flow_count = 0;
    current_avg_tank_pressure = 0;
    avg_tank_count = 0;
  }
};

// Stores values for slice and summary reports
struct ReportStore {
  float last_output_volume_report; // Last volume at which a slice report was sent
  float total_tank_output_volume; // Total volume output from the tank
  float drain_start_pressure; // The initial pressure of the drain process

  ReportStore() {
    last_output_volume_report = 0;
    total_tank_output_volume = 0;
    drain_start_pressure = 0;
  }
};

// This struct holds all of the global state
// information for the microcontroller
struct DeviceState {

  FlagStore flag;
  TimeStore time;
  TargetStore target;
  SensorStore sensor;
  SliceStore slice;
  ReportStore report;

  conf::ServicesConfig services_config;
  conf::SourceConfig source_config;
  conf::TankConfig tank_config;
  conf::FlowSensorConfig flow_sensor_config;
  conf::PressureSensorConfig pressure_sensor_config;
};

extern DeviceState env;

// Initialize global state from configs
void init_app();

// Run all necessary update functions for global state
void loop_app();

// Retrieve the volume of fluid resevoir based on the pressure
float pressure_to_volume (float pressure);

}

#endif
