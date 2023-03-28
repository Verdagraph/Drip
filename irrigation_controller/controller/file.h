#ifndef FILE_H
#define FILE_H

// Struct to hold and assign defaults for the MQTT connection
struct MQTTConfig {
  char domain[100];
  char port[10];
  char id[50];
  char username[50];
  char password[50];

  MQTTConfig() {
    strcpy(domain, MQTT_SERVER_DOMAIN_DEFAULT);
    strcpy(port, MQTT_SERVER_PORT_DEFAULT);
    strcpy(id, MQTT_ID_DEFAULT);
    strcpy(username, MQTT_USERNAME_DEFAULT);
    strcpy(password, MQTT_PASSWORD_DEFAULT);
  }
};

// Initialize file system or restart
void init_files();

// Read the MQTT config from the filesystem
MQTTConfig read_mqtt_config();

// Save the MQTT config to the filesystem. Returns true on success
bool save_mqtt_config(MQTTConfig config);

// Delete the MQTT config from the file system
//bool delete_mqtt_config();

#endif
