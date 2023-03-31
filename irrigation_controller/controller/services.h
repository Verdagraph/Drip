#ifndef SERVICES_H
#define SERVICES_H

#include "config.h"

struct DeviceState {

  bool dispense_flag;
  bool deactivate_flag;
  
  float current_output_volume;
  float target_output_volume;

  volatile int pulses;
  float pressure;

  int target_drain_time;
  float target_drain_volume;
  float target_drain_pressure;

  ServicesConfig services_config;
  InexhaustibleResevoirConfig inexhaustible_resevoir_config;
  ExhaustibleResevoirConfig exhaustible_resevoir_config;
  FlowSensorConfig flow_sensor_config;
  PressureSensorConfig pressure_sensor_config;
};

void init_state();

// Activate the device to dispense water
void dispense_activate(byte* payload, unsigned int len);

// Deactivate all current dispense or drain processes
void deactivate();

// Restart the ESP
void restart();

// Update config file
void config_change(byte* payload, unsigned int len);

// Activate the device to drain water from the exhaustible resevoir
void drain_activate(byte* payload, unsigned int len);

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
