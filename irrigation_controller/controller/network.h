#ifndef NETWORK_H
#define NETWORK_H

// Initialize WiFi, use an autoconnect AP to set wifi 
// and MQTT config if no connection can be found.
// Then initialize MQTT connection
void init_network();

// Configure WiFiManager
void setup_wifi();

// Configure MQTT PubSubClient
void setup_mqtt();

// Connect to wifi using autoconnect or on-demand fallback portal
void connect_wifi(bool auto_connect);

// Connect to MQTT server using mqtt config file
bool connect_mqtt();

// Callback for when MQTT message is received
void on_message(const char topic[], byte* payload, unsigned int len);

#endif
