// file.cpp

#include <FS.h> // File system
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson

#include "config.h"
#include "services.h"

#include "file.h"

// Initialize file system or restart
void init_files(){

  //Turn off autoformatting just in case
  SPIFFSConfig cfg;
  cfg.setAutoFormat(false);
  SPIFFS.setConfig(cfg);

  //Initialize file system. If this fails we can't store data so restart
  if(!SPIFFS.begin()){
      DEBUG_OUT.println("File system failed to mount");
      ESP.restart();
      }

}

// Read the MQTT config from the filesystem
void read_mqtt_config(MQTTConfig *config){

  // Check if config file exists
  if(!SPIFFS.exists("/mqtt_config.json")) {
    DEBUG_OUT.println("MQTT config file not found. Returning to defaults");
    return;
  }

  // Open config file
  DEBUG_OUT.println("Reading mqtt config file");
  File configFile = SPIFFS.open("/mqtt_config.json", "r");
  if (!configFile) {
    DEBUG_OUT.println("MQTT config file failed to open. Returning to defaults");
    return;
  }
  DEBUG_OUT.println("Opened mqtt config file");

  // Allocate a buffer to store contents of the file.
  size_t size = configFile.size();
  std::unique_ptr<char[]> buffer(new char[size]);
  configFile.readBytes(buffer.get(), size);
  configFile.close();

  // Assign to a JSON document
  DynamicJsonDocument json(1024);
  DeserializationError error = deserializeJson(json, buffer.get());

  // Catch deserialization error
  if (error) {
    DEBUG_OUT.print("MQTT config failed to deserialize with error: ");
    DEBUG_OUT.print(error.f_str());
    DEBUG_OUT.println(". Returning to defaults");
    return;
  }
  DEBUG_OUT.println("MQTT config serialized");
        
  // Return config struct
  const char* domain = json["domain"].as<const char*>() const;
  const char* port = json["port"].as<const char*>() const;
  const char* id = json["id".as<const char*>() const];
  const char* username = json["username"].as<const char*>() const;
  const char* password = json["password"].as<const char*>() const;

  strlcpy(config->domain, domain, sizeof(config.domain));
  strlcpy(config->port, port, sizeof(config.port));
  strlcpy(config->id, port, sizeof(config.id));
  strlcpy(config->username, username, sizeof(config.username));
  strlcpy(config->password, password, sizeof(config.password));

  return;

}

// Save the MQTT config to the filesystem
bool save_mqtt_config(MQTTConfig *config){

  DynamicJsonDocument json(1024);

  json["domain"] = config->domain;
  json["port"] = config->port;
  json["id"] = config->id;
  json["username"] = config->username;
  json["passsword"] = config->password;

  File configFile = SPIFFS.open("/mqtt_config.json", "w");
  if (!configFile){
    DEBUG_OUT.println("MQTT config file failed to open");
    return false;
  }

  serializeJson(json, configFile);
  configFile.close();
  return true;

}

// Delete the MQTT config from the file system
/*
bool delete_mqtt_config(){

  if (!SPIFFS.remove("/mqtt_config.json")){
    DEBUG_OUT.println("Failed to delete MQTT config file");
    return false;
  }

  DEBUG_OUT.println("Deleted MQTT config file");
  return true;

}
*/

void read_config(DeviceState* state){

  // Nullify unused configs
  if (!USING_INEXHAUSTIBLE_RESEVOIR_): state->inexhaustible_resevoir_config = NULL;
  if (!USING_EXHAUSTIBLE_RESEVOIR_): state->exhaustible_resevoir_config = NULL;
  if (!USING_FLOW_SENSOR_): state->flow_sensor_config = NULL;
  if (!USING_PRESSURE_SENSOR_): state->pressure_sensor_config = NULL;

  // Check if config file exists
  if(!SPIFFS.exists("/device_config.json")) {
    DEBUG_OUT.println("Main config file not found. Returning to defaults");
    return;
  }

  DEBUG_OUT.println("Reading main config file");
  File configFile = SPIFFS.open("/device_config.json", "r");
  if (!configFile) {
    DEBUG_OUT.println("Main config file failed to open. Returning to defaults");
    return;
  }
  DEBUG_OUT.println("Opened main config file");

  // Allocate a buffer to store contents of the file.
  size_t size = configFile.size();
  std::unique_ptr<char[]> buffer(new char[size]);
  configFile.readBytes(buffer.get(), size);
  configFile.close();

  StaticJsonDocument<512> json;
  DeserializationError error = deserializeJson(json, buffer.get());

  // Catch deserialization error
  if (error) {
    DEBUG_OUT.print("Main config failed to deserialize with error: ");
    DEBUG_OUT.print(error.f_str());
    DEBUG_OUT.println(". Returning to defaults");
    return;
  }
  DEBUG_OUT.println("Main config serialized");

  // Assign config values to config structs
  state->services_config->data_resolution_ml = json["services"]["res"].as<int>();

  if (USING_INEXHAUSTIBLE_RESEVOIR_) {
    state->inexhaustible_resevoir_config->static_flow_rate = json["inex"]["static_flow"].as<float>();
  }

  if (USING_EXHAUSTIBLE_RESEVOIR_) {
<<<<<<< Updated upstream
    state->exhaustible_resevoir_config.exhaustible_resevoir_timeout = json["ex"]["timeout"]..as<int>();
    state.exhaustible_resevoir_config.shape_type = json["ex"]["shape"].as<int>();
    state.exhaustible_resevoir_config.dimension_1 = json["ex"]["dim_1"].as<float>();
    state.exhaustible_resevoir_config.dimension_2 = json["ex"]["dim_2"].as<float>();
    state.exhaustible_resevoir_config.dimension_3 = json["ex"]["dim_3"].as<float>();
=======
    state->exhaustible_resevoir_config->exhaustible_resevoir_timeout = json["ex"]["timeout"].as<int>();
    state->exhaustible_resevoir_config->shape_type = json["ex"]["shape"].as<int>();
    state->exhaustible_resevoir_config->dimension_1 = json["ex"]["dim_1"].as<float>();
    state->exhaustible_resevoir_config->dimension_2 = json["ex"]["dim_2"].as<float>();
    state->exhaustible_resevoir_config->dimension_3 = json["ex"]["dim_3"].as<float>();
>>>>>>> Stashed changes
  }

  if (USING_FLOW_SENSOR_) {
    state->flow_sensor_config->pulses_per_ml = json["flow"]["ppml"].as<float>();
    state->flow_sensor_config->max_flow_rate = json["flow"]["max_flow"].as<float>();
  }

  if (USING_PRESSURE_SENSOR_) {
    state->pressure_sensor_config->use_calibration = json["pressure"]["calibration"].as<bool>();
  }

}
