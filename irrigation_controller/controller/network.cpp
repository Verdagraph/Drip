// network.cpp

#include <ESP8266WiFi.h>   //General WiFi
#include <WiFiManager.h>   // WiFiManager allows configuration through an access point
#include <PubSubClient.h>  // https://pubsubclient.knolleary.net/api
#include <ArduinoJson.h>   //https://github.com/bblanchon/ArduinoJson

#include "config.h"
#include "file.h"
#include "services.h"
#include "network.h"

WiFiClient wifi_client;  // Initialize wifi client for MQTT client to use

WiFiManager wifi_manager;  // Initialize wifi manager for autoconnect fallback

// Initialize additional MQTT parameters for autoconnect AP
WiFiManagerParameter mqtt_domain("mqtt_domain", "MQTT server domain", MQTT_SERVER_DOMAIN_DEFAULT, 100);
WiFiManagerParameter mqtt_port("mqtt_port", "MQTT server port", MQTT_SERVER_PORT_DEFAULT, 10);
WiFiManagerParameter mqtt_id("mqtt_id", "MQTT client id", MQTT_ID_DEFAULT, 50);
WiFiManagerParameter mqtt_username("mqtt_domain", "MQTT client username", MQTT_USERNAME_DEFAULT, 50);
WiFiManagerParameter mqtt_password("mqtt_password", "MQTT client password", MQTT_PASSWORD_DEFAULT, 50);

PubSubClient net::mqtt_client;      // Initialize MQTT client
conf::MQTTConfig mqtt_config;  // Initialize MQTT config

// Flag and callback to save config after configuration through WiFiManager portal
bool save_config_flag = false;
void save_callback() {
  SLOG.println("Save config flag set to true");
  save_config_flag = true;
}

void net::init_network() {

  // Assign configuration to wifi and mqtt clients
  setup_wifi();
  setup_mqtt();

  // Try to connnect to wifi. If connection fails, WiFiManager
  // will open up an access point for configuration
  connect_wifi(true);

  // Tryy MQTT connection
  // If can't connect, open up the configuration access point again
  // If the wifi is sucessfully configured, MQTT connection will try again
  // Else, after config portal timeout ESP will deep sleep and restart
  while (!connect_mqtt()) {
    connect_wifi(false);
  }
}

void net::setup_wifi() {

  wifi_manager.setDebugOutput(DEBUG);
  wifi_manager.setSaveConfigCallback(save_callback);
  wifi_manager.setClass("invert");  // Dark theme
  wifi_manager.setSTAStaticIPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);

  wifi_manager.addParameter(&mqtt_domain);
  wifi_manager.addParameter(&mqtt_port);
  wifi_manager.addParameter(&mqtt_id);
  wifi_manager.addParameter(&mqtt_username);
  wifi_manager.addParameter(&mqtt_password);

  //reset settings - for testing
  //wifi_manager.resetSettings();

  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  //wifi_manager.setTimeout(120);
}

void net::setup_mqtt() {

  mqtt_client.setClient(wifi_client);
  mqtt_client.setCallback(srvc::on_message);
}

// On failed configuration through access point, deep sleep
void on_wifi_failure() {
  SLOG.println("Failed to connect to WiFi network");
  delay(3000);
  SLOG.print("Going to sleep for ");
  SLOG.print(AP_RETRY_DELAY / (1000 * 1000 * 60));
  SLOG.println(" minutes");
  ESP.deepSleep(AP_RETRY_DELAY);
}

void net::connect_wifi(bool auto_connect) {

  // Connect to the network, and deep sleep on AP timeout
  if (auto_connect) {

    SLOG.println("Starting config portal with autoconnect");
    if (!wifi_manager.autoConnect(AP_NAME, AP_PASSWORD)) {
      on_wifi_failure();
    }

  } else {

    SLOG.println("Starting config portal on demand");
    if (!wifi_manager.startConfigPortal(AP_NAME, AP_PASSWORD)) {
      on_wifi_failure();
    }
  }
  SLOG.println("WiFi connected successfully");

  // If config save flagged, write to file, otherwise read from file
  if (save_config_flag) {
    SLOG.println("Saving MQTT config");
    strcpy(mqtt_config.domain, mqtt_domain.getValue());
    strcpy(mqtt_config.port, mqtt_port.getValue());
    strcpy(mqtt_config.id, mqtt_id.getValue());
    strcpy(mqtt_config.username, mqtt_username.getValue());
    strcpy(mqtt_config.password, mqtt_password.getValue());
    file::save_mqtt_config(&mqtt_config);
    save_config_flag = false;
  } else {
    SLOG.println("Using MQTT config from file");
    file::read_mqtt_config(&mqtt_config);
  }
}

bool net::connect_mqtt() {

  SLOG.println("Connecting to mqtt server...");
  int count = 1;
  mqtt_client.setServer(mqtt_config.domain, (int)mqtt_config.port);
  while (!mqtt_client.connect(mqtt_config.id, mqtt_config.username, mqtt_config.password)) {

    SLOG.print("*");
    delay(1000);
    if (count == MQTT_RETRY_TIMEOUT_SECONDS) {
      SLOG.println();
      SLOG.println("MQTT connection timed out");

      return false;
    }
    count++;
  }

  SLOG.println("Successfully connected to mqtt server");

  mqtt_client.subscribe(DISPENSE_ACTIVATE_TOPIC_);
  mqtt_client.subscribe(DEACTIVATE_TOPIC_);
  mqtt_client.subscribe(RESTART_TOPIC_);
  mqtt_client.subscribe(CONFIG_CHANGE_TOPIC_);
  mqtt_client.subscribe(SETTINGS_RESET_TOPIC_);

  if (USING_DRAIN_VALVE_) {
    mqtt_client.subscribe(DRAIN_ACTIVATE_TOPIC_);
  }

  char message[] = "Connected";
  srvc::publish_log(0, message);
  srvc::publish_config();

  return true;
}

void net::loop_mqtt() {
  if (mqtt_client.connected()){
    app::env.flag.mqtt_disconnect_flag = false;
    mqtt_client.loop();
  } else {
    SLOG.println("MQTT connection lost");
    app::env.flag.mqtt_disconnect_flag = true;
  }
}