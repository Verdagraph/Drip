// services.h

#ifndef SERVICES_H
#define SERVICES_H

#include <Arduino.h>

namespace srvc {

// Callback for when MQTT message is received
void on_message(const char topic[], byte* payload, unsigned int len);

// Activate the device to dispense water
void dispense_activate(byte* payload, unsigned int len);

// Deactivate all current dispense or drain processes
void deactivate();

// Restart the ESP
void restart();

// Update config file
void config_change(byte* payload, unsigned int len);

// Topic to reset WiFi, MQTT, or config settings
void settings_reset(byte* payload, unsigned int len);

// Activate the device to drain water from the exhaustible resevoir
void drain_activate(byte* payload, unsigned int len);

// Reflect the current config values on the config topic
void publish_config();

// Publish the dispensation report to the dispensation slice report topic
void publish_dispense_slice_report(bool cache, unsigned long int time, float volume, float avg_flow, float avg_pressure);

// Publish the dispensation report to the dispensation summary report topic
void publish_dispense_summary_report(unsigned long int total_time, float total_volume, float tank_volume, unsigned long int tank_time); 

// Publish the drain report to the drain report topic
void publish_drain_report();

// Publish
void publish_pressure_report(float pressure, float volume);

// Publish the given message to the log topic
void publish_log(int level, const char message[]);

}

#endif
