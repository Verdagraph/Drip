// services.h

#ifndef SERVICES_H
#define SERVICES_H

#include <Arduino.h>

#include "config.h"
#include "state.h"

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

// Publish the dispensation report to the dispensation report topic
void publish_dispense_report(int inexhaustible_resevoir_volume_output, int resevoir_switch_timestamp, int time[], int volume_output[], int avg_rate[], float avg_resevoir_pressure[], float avg_resevoir_volume[]); 

// Publish the drain report to the drain report topic
void publish_drain_report(int time[], int avg_resevoir_pressure[], int avg_resevoir_volume[]);

// Publish
void publish_pressure_report(float pressure, float volume);

// Publish the given message to the log topic
void publish_log(int level, const char message[]);

#endif
