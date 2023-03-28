#include <ArduinoJson.h>

#include "config.h";


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

}

void deactivate() {

}

void config_change(byte* payload, unsigned int len){

  StaticJsonDocument<512> json; 
  deserialize_json(payload, len, json);

}

void drain_activate(byte* payload, unsigned int len){

  StaticJsonDocument<512> json; 
  deserialize_json(payload, len, json);

}