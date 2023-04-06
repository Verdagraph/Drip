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

// Catch errors in deserialization process
void deserialize_json(byte* payload, unsigned int len, StaticJsonDocument<512>& json) {
  DeserializationError error = deserializeJson(json, payload, len);
  if (error) {
    char message[150] = "MQTT payload failed to deserialize with error: ";
    strncat(message, error.c_str(), 70);
    SLOG.println(message);
    srvc::publish_log(2, message);
  }
}

void srvc::dispense_activate(byte* payload, unsigned int len) {

  // Stop process from being overwritten
  if (app::env.flag.dispense_flag || app::env.flag.drain_flag) {
    char message[150] = "Dispense request denied, process already in progress. Deactivate with topic: ";
    strncat(message, DEACTIVATE_TOPIC_, 70);
    SLOG.println(message);
    srvc::publish_log(2, message);
    return;
  }

  // Set flags for process to start
  StaticJsonDocument<512> json;
  deserialize_json(payload, len, json);
  app::env.target.target_output_volume = json["tv"].as<float>();
  app::env.flag.dispense_flag = true;
  app::env.time.process_begin_timestamp = 0;

  char message[150];
  snprintf(message, 150, "Beginning dispensation process with target volume: %f liters", app::env.target.target_output_volume);
  SLOG.println(message);
  publish_log(0, message);
}

void srvc::publish_dispense_slice_report(unsigned long int time, float volume, float avg_flow, float avg_pressure) {

  // Catch failed connection
  if (!app::env.flag.mqtt_connected_flag) {
    SLOG.println("Unable to publish slice report. MQTT disconnected");
    return;
  }

  StaticJsonDocument<512> json;
  json["t"] = time;
  json["v"] = volume;
  json["q"] = avg_flow;

  if (USING_PRESSURE_SENSOR_ && (app::env.pressure_sensor_config.report_mode == 1 || app::env.pressure_sensor_config.report_mode == 3)) {
    json["tp"] = avg_pressure;
  }
  if (USING_PRESSURE_SENSOR_ && (app::env.pressure_sensor_config.report_mode == 2 || app::env.pressure_sensor_config.report_mode == 3)) {
    json["tv"] = app::pressure_to_volume(avg_pressure);
  }

  char buffer[256];
  size_t size = serializeJson(json, buffer);
  net::publish(DISPENSE_REPORT_SLICE_TOPIC_, buffer, size, false);


}

void srvc::publish_dispense_summary_report(unsigned long int total_time, float total_volume, float tank_volume, unsigned long int tank_time) {

  // Catch failed connection
  if (!app::env.flag.mqtt_connected_flag) {
    SLOG.println("Unable to publish summary report. Trying MQTT connection one more time.");
    if (!net::mqtt_loop()) {return;}
  }

  StaticJsonDocument<512> json;
  json["tt"] = total_time;
  json["vt"] = total_volume;

  if(USING_TANK_) {
    json["tv"] = tank_volume;
    if (USING_SOURCE_){
      json["tts"] = tank_time;
    }
  }

  char buffer[256];
  size_t size = serializeJson(json, buffer);
  net::publish(DISPENSE_REPORT_SUMMARY_TOPIC_, buffer, size, false);

}

void srvc::deactivate() {

  app::env.flag.deactivate_flag = true;
  char message[] = "Deactivation requested";
  SLOG.println(message);
  srvc::publish_log(0, message);

}

void srvc::restart() {

  char message[] = "System reset requested";
  SLOG.println(message);
  srvc::publish_log(0, message);
  ESP.restart();

}

void srvc::publish_log(int level, const char message[]) {
  StaticJsonDocument<256> json;
  json["m"] = message;
  char buffer[256];
  size_t size = serializeJson(json, buffer);
  // Switch the correct level for logging, warning, and errors
  switch (level) {
    case 0:
      net::publish(LOG_TOPIC, buffer, size, false);
    case 1:
      net::publish(WARNING_TOPIC, buffer, size, false);
    case 2:
      net::publish(ERROR_TOPIC, buffer, size, false);
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
    json["pressure"]["atmosphere"] = app::env.pressure_sensor_config.atmosphere_pressure;
  }

  char buffer[256];
  size_t size = serializeJson(json, buffer);
  net::publish(CONFIG_TOPIC_, buffer, size, true);

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

  if (USING_PRESSURE_SENSOR_ && !json["pressure"].isNull()) {
    if (!json["pressure"]["calibration"].isNull()) {
      app::env.pressure_sensor_config.use_calibration = json["pressure"]["calibration"].as<bool>();
    }
    if (!json["pressure"]["mode"].isNull()) {
      app::env.pressure_sensor_config.report_mode = json["pressure"]["mode"].as<int>();
    }
    if (!json["pressure"]["atmosphere"].isNull()) {
      app::env.pressure_sensor_config.atmosphere_pressure = json["pressure"]["atmosphere"].as<int>();
    }
  }

  file::save_config(&app::env);
  srvc::publish_config();

}

void srvc::settings_reset(byte* payload, unsigned int len) {

  StaticJsonDocument<512> json;
  deserialize_json(payload, len, json);

  if (!json["wifi_config"].isNull() && json["wifi_config"].as<bool>()) {
    net::reset_wifi_settings();
    restart();
  }

  if (!json["mqtt_config"].isNull() && json["mqtt_config"].as<bool>()) {
    net::reset_mqtt_settings();
    restart();
  }

}

void srvc::drain_activate(byte* payload, unsigned int len) {

  // Stop process from being overwritten
  if (app::env.flag.dispense_flag || app::env.flag.drain_flag) {
    char message[150] = "Drain request denied, process already in progress. Deactivate with topic: ";
    strncat(message, DEACTIVATE_TOPIC_, 70);
    SLOG.println(message);
    srvc::publish_log(2, message);
    return;
  }

  StaticJsonDocument<512> json;
  deserialize_json(payload, len, json);

  // Ensure valid input
  if ((!json["target_drain_time"].isNull() && !json["target_drain_volume"].isNull()) || (!json["target_drain_time"].isNull() && !json["target_drain_pressure"].isNull()) || (!json["target_drain_pressure"].isNull() && !json["target_drain_volume"].isNull())) {
    char message[150] = "Drain request denied, more than one target was sent";
    SLOG.println(message);
    srvc::publish_log(2, message);
    return;
  }

  bool activated = false;
  char message[150];
  if (!json["target_drain_time"].isNull()) {

    app::env.target.target_drain_time = json["target_drain_time"].as<int>();
    app::env.target.target_drain_volume = 0;
    app::env.target.target_drain_pressure = 0;
    activated = true;
    snprintf(message, 150, "Beginning drain process with target time: %f", app::env.target.target_drain_time);

  } else if (!json["target_drain_volume"].isNull()) {

    if (USING_PRESSURE_SENSOR_) {

      app::env.target.target_drain_time = 0;
      app::env.target.target_drain_volume = json["target_drain_volume"].as<float>();
      app::env.target.target_drain_pressure = 0;
      activated = true;

    } else {
      char message[] = "Unable to set target drain volume as tank pressure sensor not active";
      SLOG.println(message);
      publish_log(2, message);
      return;
      snprintf(message, 150, "Beginning drain process with target volume: %f", app::env.target.target_drain_volume);
    }

  } else if (!json["target_drain_pressure"].isNull()) {

    if (USING_PRESSURE_SENSOR_) {

      app::env.target.target_drain_time = 0;
      app::env.target.target_drain_volume = 0;
      app::env.target.target_drain_pressure = json["target_drain_pressure"].as<float>();
      activated = true;
      snprintf(message, 150, "Beginning drain process with target pressure: %f", app::env.target.target_drain_pressure);

    } else {
      char message[] = "Unable to set target drain volume as tank pressure sensor not active";
      SLOG.println(message);
      publish_log(2, message);
      return;
    }

  }

    if (activated) {
      app::env.flag.drain_flag = true;
      app::env.time.process_begin_timestamp = 0;
      SLOG.println(message);
      publish_log(0, message);
    }
}

void srvc::publish_drain_summary_report(unsigned long int total_time, float start_pressure, float end_pressure, float start_volume, float end_volume) {

  // Catch failed connection
  if (!app::env.flag.mqtt_connected_flag) {
    SLOG.println("Unable to publish report. MQTT disconnected");
    return;
  }

  StaticJsonDocument<512> json;
  json["tt"] = total_time;

  if (USING_PRESSURE_SENSOR_) {
    json["sp"] = start_pressure;
    json["ep"] = end_pressure;
    json["sv"] = start_volume;
    json["ev"] = end_volume;
  }

  char buffer[256];
  size_t size = serializeJson(json, buffer);
  net::publish(DRAIN_REPORT_SUMMARY_TOPIC_, buffer, size, false);

}