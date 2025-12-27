// file.cpp

#include <FS.h>           // File system
#include <ArduinoJson.h>  //https://github.com/bblanchon/ArduinoJson

#include "config.h"
#include "app.h"

#include "file.h"

void file::init_files() {

  //Turn off autoformatting just in case
  SPIFFSConfig cfg;
  cfg.setAutoFormat(false);
  SPIFFS.setConfig(cfg);

  //Initialize file system. If this fails we can't store data so restart
  SLOG.println("Initializing file system");
  if (!SPIFFS.begin()) {
    SLOG.println("File system failed to mount");
    ESP.restart();
  }
}

void file::read_mqtt_config(conf::MQTTConfig* config) {

  SLOG.println("Reading MQTT config file");

  // Check if config file exists
  if (!SPIFFS.exists("/mqtt_config.json")) {
    SLOG.println("MQTT config file not found. Returning to defaults");
    return;
  }

  // Open config file
  File configFile = SPIFFS.open("/mqtt_config.json", "r");
  if (!configFile) {
    SLOG.println("MQTT config file failed to open. Returning to defaults");
    return;
  }
  SLOG.println("Opened mqtt config file");

  // Allocate a buffer to store contents of the file.
  size_t size = configFile.size();
  std::unique_ptr<char[]> buffer(new char[size]);
  configFile.readBytes(buffer.get(), size);
  configFile.close();

  // Assign to a JSON document
  StaticJsonDocument<512> json;
  DeserializationError error = deserializeJson(json, buffer.get());

  // Catch deserialization error
  if (error) {
    SLOG.print("MQTT config failed to deserialize with error: ");
    SLOG.print(error.f_str());
    SLOG.println(". Returning to defaults");
    return;
  }
  SLOG.println("MQTT config serialized");

  // Return config struct
  const char* domain = json["domain"].as<const char*>();
  const char* port = json["port"].as<const char*>();
  const char* id = json["id"].as<const char*>();
  const char* username = json["username"].as<const char*>();
  const char* password = json["password"].as<const char*>();

  strlcpy(config->domain, domain, sizeof(config->domain));
  strlcpy(config->port, port, sizeof(config->port));
  strlcpy(config->id, port, sizeof(config->id));
  strlcpy(config->username, username, sizeof(config->username));
  strlcpy(config->password, password, sizeof(config->password));

  SLOG.println("MQTT config read sucessfully");
  return;
}

bool file::save_mqtt_config(conf::MQTTConfig* config) {

  SLOG.println("Saving MQTT config to file");

  StaticJsonDocument<512> json;

  // Assign config values to json document
  json["domain"] = config->domain;
  json["port"] = config->port;
  json["id"] = config->id;
  json["username"] = config->username;
  json["passsword"] = config->password;

  // Open config file
  File configFile = SPIFFS.open("/mqtt_config.json", "w");
  if (!configFile) {
    SLOG.println("MQTT config file failed to open");
    return false;
  }

  // Write to file
  serializeJson(json, configFile);
  configFile.close();
  SLOG.println("MQTT config written to file");
  return true;
}

bool file::delete_mqtt_config(){

  if (!SPIFFS.remove("/mqtt_config.json")){
    SLOG.println("Failed to delete MQTT config file");
    return false;
  }

  SLOG.println("Deleted MQTT config file");
  return true;

}

void file::read_config(app::DeviceState* state) {

  SLOG.println("Reading device config from file");

  // Check if config file exists
  if (!SPIFFS.exists("/device_config.json")) {
    SLOG.println("Main config file not found. Returning to defaults");
    return;
  }

  // Open file
  File configFile = SPIFFS.open("/device_config.json", "r");
  if (!configFile) {
    SLOG.println("Main config file failed to open. Returning to defaults");
    return;
  }
  SLOG.println("Opened main config file");

  // Allocate a buffer to store contents of the file.
  size_t size = configFile.size();
  std::unique_ptr<char[]> buffer(new char[size]);
  configFile.readBytes(buffer.get(), size);
  configFile.close();

  StaticJsonDocument<512> json;
  DeserializationError error = deserializeJson(json, buffer.get());

  // Catch deserialization error
  if (error) {
    SLOG.print("Main config failed to deserialize with error: ");
    SLOG.print(error.f_str());
    SLOG.println(". Returning to defaults");
    return;
  }
  SLOG.println("Main config serialized");

  // Assign config values to config structs
  state->services_config.data_resolution_l = json["srvc"]["res"].as<float>();
  if (USING_SOURCE_) {
    state->source_config.static_flow_rate = json["src"]["rate"].as<float>();
  }
  if (USING_TANK_) {
    state->tank_config.tank_timeout = json["tnk"]["time"].as<int>();
    state->tank_config.shape_type = json["tnk"]["shape"].as<int>();
    state->tank_config.dimension_1 = json["tnk"]["dim1"].as<float>();
    state->tank_config.dimension_2 = json["tnk"]["dim2"].as<float>();
    state->tank_config.dimension_3 = json["tnk"]["dim3"].as<float>();
  }
  if (USING_FLOW_SENSOR_) {
    state->flow_sensor_config.pulses_per_l = json["flow"]["ppl"].as<float>();
    state->flow_sensor_config.max_flow_rate = json["flow"]["max"].as<float>();
    state->flow_sensor_config.min_flow_rate = json["flow"]["min"].as<float>();
    state->flow_sensor_config.calibration_timeout = json["flow"]["ctime"].as<int>();
    state->flow_sensor_config.calibration_max_volume = json["flow"]["cmax"].as<float>();
  }
  if (USING_PRESSURE_SENSOR_) {
    state->pressure_sensor_config.report_mode = json["prssr"]["mode"].as<int>();
    state->pressure_sensor_config.atmosphere_pressure = json["prssr"]["atmo"].as<float>();
  }

  SLOG.println("Main config read sucessfully");

}

bool file::save_config(app::DeviceState * state) {

  StaticJsonDocument<512> json;

  // Assign config values to json document
  json["srvc"]["res"] = state->services_config.data_resolution_l;
  if (USING_SOURCE_) {
    json["src"]["rate"] = state->source_config.static_flow_rate;
  }
  if (USING_TANK_) {
    json["tnk"]["time"] = state->tank_config.tank_timeout;
    json["tnk"]["shape"] = state->tank_config.shape_type;
    json["tnk"]["dim1"] = state->tank_config.dimension_1;
    json["tnk"]["dim2"] = state->tank_config.dimension_2;
    json["tnk"]["dim3"] = state->tank_config.dimension_3;
  }
  if (USING_FLOW_SENSOR_) {
    json["flow"]["ppl"] = state->flow_sensor_config.pulses_per_l;
    json["flow"]["max"] = state->flow_sensor_config.max_flow_rate;
    json["flow"]["min"] = state->flow_sensor_config.min_flow_rate;
    json["flow"]["ctime"] = state->flow_sensor_config.calibration_timeout;
    json["flow"]["cmax"] = state->flow_sensor_config.calibration_max_volume;
  }
  if (USING_PRESSURE_SENSOR_) {
    json["prssr"]["mode"] = state->pressure_sensor_config.report_mode;
    json["prssr"]["atmo"] = state->pressure_sensor_config.atmosphere_pressure;
  }

  // Open file
  File configFile = SPIFFS.open("/device_config.json", "w");
  if (!configFile) {
    SLOG.println("Config file failed to open");
    return false;
  }

  // Write to file
  serializeJson(json, configFile);
  configFile.close();
  SLOG.println("MQTT config written to file");
  return true;
}