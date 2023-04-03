// state.h

#ifndef STATE_H
#define STATE_H

// Stores flags on process states
struct FlagStore {
  bool dispense_flag; // True if dispensing
  bool drain_flag; // True if draining
  bool deactivate_flag; // True if pending deactivation
  bool resevoir_switch_flag; // True if dispensing and resevoir has been switched

  FlagStore() {
    dispense_flag = false;
    drain_flag = false;
    deactivate_flag = false;
    resevoir_switch_flag = false;
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
  float last_pressure; // Previous timestamp pressure

  SensorStore() {
    pulses = 0;
    last_pulses = 0;
    pressure = 0;
    last_pressure = 0;
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

  float current_avg_rate; // Current total flow rate between reports
  int avg_flow_count; // Number of flow rate measurments between reports
  float current_avg_tank_pressure; // Current tank pressure between reports
  float current_avg_tank_volume; // Current tank volume between reports
  int avg_tank_count; // Number of tank pressure measurments between reports

  SliceStore() {
    time_elapsed = 0;
    pulses_elapsed = 0;
    output_volume_elapsed = 0;
    flow_rate = 0;
    total_time_elapsed = 0;
    total_output_volume = 0;
    current_avg_rate = 0;
    avg_flow_count = 0;
    current_avg_tank_pressure = 0;
    current_avg_tank_volume = 0;
    avg_tank_count = 0;
  }
};

// Stores values for slice and summary reports
struct ReportStore {
  float last_output_volume_report; // Last volume at which a slice report was sent
  float total_tank_output_volume; // Total volume output from the tank
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

  ServicesConfig services_config;
  SourceConfig source_config;
  TankConfig tank_config;
  FlowSensorConfig flow_sensor_config;
  PressureSensorConfig pressure_sensor_config;
};

extern DeviceState global;

// Initialize global state from configs
void init_state();

// Run all necessary update functions for global state
void loop_state();


/*
// Return the height of fluid in meters for a given pressure
#define DENSITY_GRAVITY (9.807 * 997)
float pressure_to_height(float pressure) {
    return pressure / DENSITY_GRAVITY;
}
*/

/*
#define PI 3.1415926535897932384626433832795
float height_to_volume(ExhaustibleResevoirConfig exhaustible_resevoir_config, float height) {
  switch (shape_type) {
    case 1: // Rectangular prism: length, width, height
      return exhaustible_resevoir_config.dimension_1 * exhaustible_resevoir_config.dimension_2 * height;
    case 2: // Cylinder: radius, height, N/A
      return PI * exhaustible_resevoir_config.dimension_1 * exhaustible_resevoir_config.dimension_1 * height;
    default:
      return NULL;
  }
}
*/

#endif
