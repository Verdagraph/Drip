// services.cpp

#include <ArduinoJson.h>

#include "config.h"
#include "file.h"

#include "services.h"

DeviceState state;

void init_state(){
  read_config(&state);
}

void deserialize_json(byte* payload, unsigned int len, StaticJsonDocument<512>& json){
    DeserializationError error = deserializeJson(json, payload, len);
    if (error) {
        DEBUG_OUT.print("MQTT payload failed to deserialize with error: ");
        DEBUG_OUT.println(error.f_str());
    }
}

void dispense_activate(byte* payload, unsigned int len){

  StaticJsonDocument<512> json; 
  deserialize_json(payload, len, json);

  state.target_output_volume = json["target_volume"].as<int>();

  

}

void deactivate() {

  state.deactivate_flag = true;

}

void restart() {

  // Output restart notice to status

  ESP.restart();
  
}

void config_change(byte* payload, unsigned int len){

  StaticJsonDocument<512> json; 
  deserialize_json(payload, len, json);

}

void drain_activate(byte* payload, unsigned int len){

  StaticJsonDocument<512> json; 
  deserialize_json(payload, len, json);

  if (json["target_drain_time"]) {
    state.target_drain_time = json["target_drain_time"].as<int>();
    state.target_drain_volume = NULL;
    state.target_drain_pressure = NULL;
  } else if (json["target_drain_volume"]) {
    if (USING_PRESSURE_SENSOR_) {
        state.target_drain_time = NULL;
        state.target_drain_volume = json["target_drain_volume"].as<float>();
        state.target_drain_pressure = NULL;
      } else {
        // Send error to status  
      }
  } else if (json["target_drain_pressure"]) {
    if (USING_PRESSURE_SENSOR_) {
        state.target_drain_time = NULL;
        state.target_drain_volume = NULL;
        state.target_drain_pressure = json["target_drain_pressure"].as<float>();
      } else {
        // Send error to status  
      }
  }

}
