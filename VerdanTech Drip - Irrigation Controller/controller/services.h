// services.h

#ifndef SERVICES_H
#define SERVICES_H

#include <Arduino.h>

namespace srvc {

// Callback for when MQTT message is received
void on_message(const char topic[], byte* payload, unsigned int len);

// Wrap the functionality for printing messages to serial and MQTT
void info(char message[]);
void warning(char message[]);
void error(char message[]);

// Activate the device to dispense water
void dispense_activate(byte* payload, unsigned int len);

// Publish the dispensation report to the dispensation slice report topic
void publish_dispense_slice_report(unsigned long int time, float volume, float avg_flow, float avg_pressure);

// Publish the dispensation report to the dispensation summary report topic
void publish_dispense_summary_report(unsigned long int total_time, float total_volume, float tank_volume, unsigned long int tank_time); 

// Deactivate all current dispense or drain processes
void deactivate();

// Restart the ESP
void restart();

// Publish the given message to the log topic
void publish_log(int level, const char message[]);

// Reflect the current config values on the config topic
void publish_config();

// Update config file
void config_change(byte* payload, unsigned int len);

// Topic to reset WiFi, or MQTT settings
void settings_reset(byte* payload, unsigned int len);

// Begin the flow calibration process
void flow_calibration_begin(byte* payload, unsigned int len);

// Request the flow calibration process dispense some volume
void flow_calibration_dispense(byte* payload, unsigned int len);

// Send the volume measurement to the calibration process
void flow_calibration_measure(byte* payload, unsigned int len);

// Activate the device to drain water from the exhaustible resevoir
void drain_activate(byte* payload, unsigned int len);

// Publish the drain report to the drain report topic
void publish_drain_summary_report(unsigned long int total_time, float start_pressure, float end_pressure, float start_volume, float end_volume);

// Publish
void publish_pressure_report();


}

#endif
