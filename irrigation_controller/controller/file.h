// file.h

#ifndef FILE_H
#define FILE_H

#include "config.h"
#include "app.h"

namespace file {

// Initialize file system or restart
void init_files();

// Read the MQTT config from the filesystem
void read_mqtt_config(conf::MQTTConfig *config);

// Save the MQTT config to the filesystem. Returns true on success
bool save_mqtt_config(conf::MQTTConfig *config);

// Delete the MQTT config from the file system
//bool delete_mqtt_config();

// Read the config from the file system
// into the device state
void read_config(app::DeviceState *state);

// Save given device configuration into file
bool save_config(app::DeviceState *state);

}

#endif
