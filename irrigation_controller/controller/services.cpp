// services.cpp

#include <ArduinoJson.h>

#include "config.h"
#include "file.h"
#include "network.h"
#include "app.h"

#include "services.h"

void srvc::on_message(const char topic[], byte* payload, unsigned int len) {

  SLOG.print("Recieved a message in: ");
  SLOG.println(topic);

  // Store whether recieved topic has been handled
  bool handled_topic = false;

  if (strcmp(topic, DISPENSE_ACTIVATE_TOPIC_) == 0) {
    dispense_activate(payload, len);
    handled_topic = true;
  }

  if (strcmp(topic, DEACTIVATE_TOPIC_) == 0) {
    deactivate();
    handled_topic = true;
  }

  if (strcmp(topic, RESTART_TOPIC_) == 0) {
    restart();
    handled_topic = true;
  }

  if (strcmp(topic, CONFIG_CHANGE_TOPIC_) == 0) {
    config_change(payload, len);
    handled_topic = true;
  }

  if (strcmp(topic, SETTINGS_RESET_TOPIC_) == 0) {
    settings_reset(payload, len);
    handled_topic = true;
  }

  if (USING_DRAIN_VALVE_ && strcmp(topic, DRAIN_ACTIVATE_TOPIC_) == 0) {
    drain_activate(payload, len);
    handled_topic = true;
  }

  if (!handled_topic) {
    SLOG.println("Topic is unhandled");
  }
}

void deserialize_json(byte* payload, unsigned int len, StaticJsonDocument<512>& json) {
  DeserializationError error = deserializeJson(json, payload, len);
  if (error) {
    //const char error_msg[200] = error.f_str();
    //SLOG.print("MQTT payload failed to deserialize with error: ");
    //SLOG.println(error_msg);
    //srvc::publish_log("error", "MQTT payload failed to deserialize with error: " + error_msg)
  }
}

void srvc::dispense_activate(byte* payload, unsigned int len) {

  //
  if (app::env.flag.dispense_flag || app::env.flag.drain_flag) {
    //SLOG.println("Dispense request denied, dispense process already in progress. Deactivate with topic: " + DEACTIVATE_TOPIC_);
    //srvc::publish_log("error", "Dispense request denied, process already in progress. Deactivate with topic: " + DEACTIVATE_TOPIC_);
    return;
  }

  StaticJsonDocument<512> json;
  deserialize_json(payload, len, json);

  app::env.target.target_output_volume = json["target_volume"].as<int>();
  app::env.flag.dispense_flag = true;
  app::env.time.process_begin_timestamp = 0;

  //char message[] = 
  // Beginning dispensation process with target volume: 
}

void srvc::deactivate() {

  app::env.flag.deactivate_flag = true;
  char message[] = "Deactivated processes";
  SLOG.println(message);
  srvc::publish_log(0, message);
}

void srvc::restart() {

  // Output restart notice to status
  char message[] = "System reset requested";
  SLOG.println(message);
  srvc::publish_log(0, message);
  ESP.restart();
}

void srvc::config_change(byte* payload, unsigned int len) {

  StaticJsonDocument<512> json;
  deserialize_json(payload, len, json);

  if (!json["services"].isNull()) {
    if (!json["services"]["data_resolution_l"].isNull()) {
      app::env.services_config.data_resolution_l = json["services"]["data_resolution_l"].as<int>();
    }
  }

  if (USING_SOURCE_ && !json["source"].isNull()) {
    if (!json["source"]["static_flow_rate"].isNull()) {
      app::env.source_config.static_flow_rate = json["source"]["static_flow_rate"].as<float>();
    }
  }

  if (USING_TANK_ && !json["tank"].isNull()) {
    if (!json["tank"]["timeout"].isNull()) {
      app::env.tank_config.tank_timeout = json["tank"]["timeout"].as<int>();
    }
    if (!json["tank"]["shape"].isNull()) {
      app::env.tank_config.shape_type = json["tank"]["shape"].as<float>();
    }
    if (!json["tank"]["dim_1"].isNull()) {
      app::env.tank_config.dimension_1 = json["tank"]["dim_1"].as<int>();
    }
    if (!json["tank"]["dim_2"].isNull()) {
      app::env.tank_config.dimension_2 = json["tank"]["dim_2"].as<float>();
    }
    if (!json["tank"]["dim_3"].isNull()) {
      app::env.tank_config.dimension_3 = json["tank"]["dim_3"].as<float>();
    }
  }

  if (USING_FLOW_SENSOR_ && !json["flow"].isNull()) {
    if (!json["flow"]["pulses_per_l"].isNull()) {
      app::env.flow_sensor_config.pulses_per_l = json["flow"]["pulses_per_l"].as<float>();
    }
    if (!json["flow"]["max_flow_rate"].isNull()) {
      app::env.flow_sensor_config.max_flow_rate = json["flow"]["max_flow_rate"].as<float>();
    }
    if (!json["flow"]["min_flow_rate"].isNull()) {
      app::env.flow_sensor_config.min_flow_rate = json["flow"]["min_flow_rate"].as<float>();
    }
  }

  if (USING_PRESSURE_SENSOR_ && !json["flow"].isNull()) {
    if (!json["pressure"]["calibration"].isNull()) {
      app::env.pressure_sensor_config.use_calibration = json["pressure"]["calibration"].as<bool>();
    }
    if (!json["pressure"]["calibration"].isNull()) {
      app::env.pressure_sensor_config.report_mode = json["pressure"]["mode"].as<int>();
    }
  }

  file::save_config(&app::env);
  srvc::publish_config();
}

void srvc::settings_reset(byte* payload, unsigned int len) {

  StaticJsonDocument<512> json;
  deserialize_json(payload, len, json);
}

void srvc::drain_activate(byte* payload, unsigned int len) {

  StaticJsonDocument<512> json;
  deserialize_json(payload, len, json);

  if (json["target_drain_time"]) {
    app::env.target.target_drain_time = json["target_drain_time"].as<int>();
    app::env.target.target_drain_volume = 0;
    app::env.target.target_drain_pressure = 0;
  } else if (json["target_drain_volume"]) {
    if (USING_PRESSURE_SENSOR_) {
      app::env.target.target_drain_time = 0;
      app::env.target.target_drain_volume = json["target_drain_volume"].as<float>();
      app::env.target.target_drain_pressure = 0;
    } else {
      // Send error to status
    }
  } else if (json["target_drain_pressure"]) {
    if (USING_PRESSURE_SENSOR_) {
      app::env.target.target_drain_time = 0;
      app::env.target.target_drain_volume = 0;
      app::env.target.target_drain_pressure = json["target_drain_pressure"].as<float>();
    } else {
      // Send error to status
    }
  }
}

void srvc::publish_config() {

  StaticJsonDocument<512> json;

  json["services"]["data_resolution_l"] = app::env.services_config.data_resolution_l;

  if (USING_SOURCE_) {
    json["source"]["static_flow_rate"] = app::env.source_config.static_flow_rate;
  }

  if (USING_TANK_) {
    json["tank"]["timeout"] = app::env.tank_config.tank_timeout;
    json["tank"]["shape"] = app::env.tank_config.shape_type;
    json["tank"]["dim_1"] = app::env.tank_config.dimension_1;
    json["tank"]["dim_2"] = app::env.tank_config.dimension_2;
    json["tank"]["dim_3"] = app::env.tank_config.dimension_3;
  }

  if (USING_FLOW_SENSOR_) {
    json["flow"]["pulses_per_l"] = app::env.flow_sensor_config.pulses_per_l;
    json["flow"]["max_flow_rate"] = app::env.flow_sensor_config.max_flow_rate;
    json["flow"]["min_flow_rate"] = app::env.flow_sensor_config.min_flow_rate;
  }

  if (USING_PRESSURE_SENSOR_) {
    json["pressure"]["calibration"] = app::env.pressure_sensor_config.use_calibration;
    json["pressure"]["mode"] = app::env.pressure_sensor_config.report_mode;
  }

  char buffer[256];
  size_t size = serializeJson(json, buffer);
  //net::mqtt_client.publish(CONFIG_TOPIC_, buffer, size, true);

}

void srvc::publish_dispense_slice_report(bool cache, unsigned long int time, float volume, float avg_flow, float avg_pressure) {

  StaticJsonDocument<512> json;

  json["time"] = time;
  json["vol"] = volume;
  json["rate"] = avg_flow;

  if (USING_PRESSURE_SENSOR_) {
    json["pressure"] = avg_pressure;
  }

  char buffer[256];
  size_t size = serializeJson(json, buffer);

  if (cache) {



  } else {

  }


}

void srvc::publish_dispense_summary_report(unsigned long int total_time, float total_volume, float tank_volume, unsigned long int tank_time) {

}

void srvc::publish_log(int level, const char message[]) {
  StaticJsonDocument<256> json;
  json["message"] = message;
  char buffer[256];
  size_t size = serializeJson(json, buffer);
  //switch (level) {
    //case 0:
      //net::publish(LOG_TOPIC, buffer, size, false);
    //case 1:
      //net::publish(ERROR_TOPIC, buffer, size, false);
  //}
}
