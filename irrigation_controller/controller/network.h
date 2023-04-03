// network.h

#ifndef NETWORK_H
#define NETWORK_H

// Initialize WiFi, use an autoconnect AP to set wifi 
// and MQTT config if no connection can be found.
// Then initialize MQTT connection
// And return to fallback portal on MQTT connection failure
void init_network();

// Configure WiFiManager
void setup_wifi();

// Configure MQTT PubSubClient
void setup_mqtt();

// Connect to wifi using autoconnect or on-demand fallback portal
void connect_wifi(bool auto_connect);

// Connect to MQTT server using mqtt config file
bool connect_mqtt();

// Publish message to MQTT client
void publish(const char topic[], const char message[], size_t size, bool retain);

#endif
