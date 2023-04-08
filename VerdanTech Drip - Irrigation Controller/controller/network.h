// network.h

#ifndef NETWORK_H
#define NETWORK_H

#include <PubSubClient.h>  // https://pubsubclient.knolleary.net/api

namespace net {

// Initialize WiFi, use an autoconnect AP to set wifi 
// and MQTT config if no connection can be found.
// Then initialize MQTT connection
// And return to fallback portal on MQTT connection failure
void init_network();

// Recieve new MQTT messages and handle disconnect
void loop_mqtt();

// Handle reconnection
void reconnect_mqtt();

// Publish message to MQTT client
void publish(const char topic[], char message[], size_t size, bool retain);

// Reset the WifiManager settings to default
void reset_wifi_settings();

// Reset the MQTT client settings to default
void reset_mqtt_settings();

}

#endif
