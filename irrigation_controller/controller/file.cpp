#include <FS.h> // File system
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson

#include "config.h"

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
MQTTConfig read_mqtt_config(){

  MQTTConfig config;

  // Check if config file exists
  if(!SPIFFS.exists("/mqtt_config.json")) {
    DEBUG_OUT.println("MQTT config file not found. Returning to defaults");
    return config;
  }

  // Open config file
  DEBUG_OUT.println("Reading config file");
  File configFile = SPIFFS.open("/mqtt_config.json", "r");
  if (!configFile) {
    DEBUG_OUT.println("MQTT config file failed to open. Returning to defaults");
    return config;
  }
  DEBUG_OUT.println("Opened config file");

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
    return config;
  }
  DEBUG_OUT.println("MQTT config serialized");
        
  // Return config struct
  const char* domain = json["domain"];
  const char* port = json["port"];
  const char* id = json["id"];
  const char* username = json["username"];
  const char* password = json["password"];

  strlcpy(config.domain, domain, sizeof(config.domain));
  strlcpy(config.port, port, sizeof(config.port));
  strlcpy(config.id, port, sizeof(config.id));
  strlcpy(config.username, username, sizeof(config.username));
  strlcpy(config.password, password, sizeof(config.password));

  return config;

}

// Save the MQTT config to the filesystem
bool save_mqtt_config(MQTTConfig config){

  DynamicJsonDocument json(1024);

  json["domain"] = config.domain;
  json["port"] = config.port;
  json["id"] = config.id;
  json["username"] = config.username;
  json["passsword"] = config.password;

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