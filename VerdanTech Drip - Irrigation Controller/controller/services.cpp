// services.cpp

#include <ArduinoJson.h>

#include "config.h"
#include "file.h"
#include "pins.h"
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
    app::deactivate();
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

  if (strcmp(topic, FLOW_SENSOR_CALIBRATE_BEGIN_TOPIC_) == 0) {
    flow_calibration_begin(payload, len);
    handled_topic = true;
  }

  if (strcmp(topic, FLOW_SENSOR_CALIBRATE_DISPENSE_TOPIC_) == 0) {
    flow_calibration_dispense(payload, len);
    handled_topic = true;
  }

  if (strcmp(topic, FLOW_SENSOR_CALIBRATE_MEASURE_TOPIC_) == 0) {
    flow_calibration_measure(payload, len);
    handled_topic = true;
  }

  if (USING_DRAIN_VALVE_ && strcmp(topic, DRAIN_ACTIVATE_TOPIC_) == 0) {
    drain_activate(payload, len);
    handled_topic = true;
  }

  if (USING_PRESSURE_SENSOR_ && strcmp(topic, PRESSURE_REQUEST_TOPIC_) == 0) {
    srvc::publish_pressure_report();
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
  if (app::env.flag.dispense_flag || app::env.flag.drain_flag || app::env.flag.calibration_flag) {
    char message[150] = "Dispense request denied, process already in progress. Deactivate with topic: ";
    strncat(message, DEACTIVATE_TOPIC_, 70);
    SLOG.println(message);
    srvc::publish_log(2, message);
    return;
  }


  // Set flags for process to start
  StaticJsonDocument<512> json;
  deserialize_json(payload, len, json);

  // Validate input 
  if (json["tv"].isNull()) {
    char message[] = "Dispense requested without a target volume";
    SLOG.println(message);
    srvc::publish_log(2, message);
    return;
  }

  app::open_dispense_process(json["tv"].as<float>());
  srvc::publish_pressure_report();

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
  json["t"] = (float)time / 1000;
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
    SLOG.println("Unable to publish dispense summary report. Trying MQTT connection one more time.");
    if (!net::loop_mqtt()) {return;}
  }

  StaticJsonDocument<512> json;
  json["tt"] = (float)total_time / 1000;
  json["vt"] = total_volume;

  if(USING_TANK_) {
    json["tv"] = tank_volume;
    if (USING_SOURCE_){
      json["tts"] = (float)tank_time / 1000;
    }
  }

  char buffer[256];
  size_t size = serializeJson(json, buffer);
  net::publish(DISPENSE_REPORT_SUMMARY_TOPIC_, buffer, size, false);
  srvc::publish_pressure_report();

}

void srvc::restart() {

  char message[] = "System reset requested";
  SLOG.println(message);
  srvc::publish_log(0, message);
  app::restart();

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

  json["srvc"]["res"] = app::env.services_config.data_resolution_l;

  if (USING_SOURCE_) {
    json["src"]["rate"] = app::env.source_config.static_flow_rate;
  }

  if (USING_TANK_) {
    json["tnk"]["time"] = app::env.tank_config.tank_timeout;
    json["tnk"]["shape"] = app::env.tank_config.shape_type;
    json["tnk"]["dim1"] = app::env.tank_config.dimension_1;
    json["tnk"]["dim2"] = app::env.tank_config.dimension_2;
    json["tnk"]["dim3"] = app::env.tank_config.dimension_3;
  }

  if (USING_FLOW_SENSOR_) {
    json["flow"]["ppl"] = app::env.flow_sensor_config.pulses_per_l;
    json["flow"]["max"] = app::env.flow_sensor_config.max_flow_rate;
    json["flow"]["min"] = app::env.flow_sensor_config.min_flow_rate;
    json["flow"]["ctime"] = app::env.flow_sensor_config.calibration_timeout;
    json["flow"]["cmax"] = app::env.flow_sensor_config.calibration_max_volume;
  }

  if (USING_PRESSURE_SENSOR_) {
    json["prssr"]["mode"] = app::env.pressure_sensor_config.report_mode;
    json["prssr"]["atmo"] = app::env.pressure_sensor_config.atmosphere_pressure;
  }

  char buffer[256];
  size_t size = serializeJson(json, buffer);
  net::publish(CONFIG_TOPIC_, buffer, size, true);

}

void srvc::config_change(byte* payload, unsigned int len) {

  StaticJsonDocument<512> json;
  deserialize_json(payload, len, json);

  if (!json["srvc"].isNull()) {
    if (!json["srvc"]["res"].isNull()) {
      app::env.services_config.data_resolution_l = json["srvc"]["res"].as<int>();
    }
  }

  if (USING_SOURCE_ && !json["src"].isNull()) {
    if (!json["src"]["rate"].isNull()) {
      app::env.source_config.static_flow_rate = json["src"]["rate"].as<float>();
    }
  }

  if (USING_TANK_ && !json["tnk"].isNull()) {
    if (!json["tnk"]["time"].isNull()) {
      app::env.tank_config.tank_timeout = json["tnk"]["time"].as<int>();
    }
    if (!json["tnk"]["shape"].isNull()) {
      app::env.tank_config.shape_type = json["tnk"]["shape"].as<float>();
    }
    if (!json["tnk"]["dim1"].isNull()) {
      app::env.tank_config.dimension_1 = json["tnk"]["dim1"].as<int>();
    }
    if (!json["tnk"]["dim2"].isNull()) {
      app::env.tank_config.dimension_2 = json["tnk"]["dim2"].as<float>();
    }
    if (!json["tnk"]["dim3"].isNull()) {
      app::env.tank_config.dimension_3 = json["tnk"]["dim3"].as<float>();
    }
  }

  if (USING_FLOW_SENSOR_ && !json["flow"].isNull()) {
    if (!json["flow"]["ppl"].isNull()) {
      app::env.flow_sensor_config.pulses_per_l = json["flow"]["ppl"].as<float>();
    }
    if (!json["flow"]["max"].isNull()) {
      app::env.flow_sensor_config.max_flow_rate = json["flow"]["max"].as<float>();
    }
    if (!json["flow"]["min"].isNull()) {
      app::env.flow_sensor_config.min_flow_rate = json["flow"]["min"].as<float>();
    }
    if (!json["flow"]["ctime"].isNull()) {
      app::env.flow_sensor_config.calibration_timeout = json["flow"]["ctime"].as<float>();
    }
    if (!json["flow"]["cmax"].isNull()) {
      app::env.flow_sensor_config.calibration_max_volume = json["flow"]["cmax"].as<float>();
    }
  }

  if (USING_PRESSURE_SENSOR_ && !json["prssr"].isNull()) {
    if (!json["prssr"]["mode"].isNull()) {
      app::env.pressure_sensor_config.report_mode = json["prssr"]["mode"].as<int>();
    }
    if (!json["prssr"]["atmo"].isNull()) {
      app::env.pressure_sensor_config.atmosphere_pressure = json["prssr"]["atmo"].as<int>();
    }
  }

  file::save_config(&app::env);
  srvc::publish_config();

}

void srvc::settings_reset(byte* payload, unsigned int len) {

  StaticJsonDocument<512> json;
  deserialize_json(payload, len, json);

  if (!json["wifi"].isNull() && json["wifi"].as<bool>()) {
    net::reset_wifi_settings();
    restart();
  }

  if (!json["mqtt"].isNull() && json["mqtt"].as<bool>()) {
    net::reset_mqtt_settings();
    restart();
  }

}

void srvc::flow_calibration_begin(byte* payload, unsigned int len) {

  // Stop process from being overwritten
  if (app::env.flag.dispense_flag || app::env.flag.drain_flag) {
    char message[150] = "Calibration request denied, process already in progress. Deactivate with topic: ";
    strncat(message, DEACTIVATE_TOPIC_, 70);
    SLOG.println(message);
    srvc::publish_log(2, message);
    return;
  }

  StaticJsonDocument<512> json;
  deserialize_json(payload, len, json);

  // Ensure valid input
  if (json["id"].isNull()) {
    char message[150] = "Calibration request denied, no ID was sent";
    SLOG.println(message);
    srvc::publish_log(2, message);
    return;
  }
  int id = json["id"].as<int>();

  if (app::env.flag.calibration_flag) {

    if (id == app::env.report.calibration_id) {

      app::close_flow_calibration_process(true);

    } else {
      char message[150] = "Calibration request denied, calibration is already in progess. Cancel with a correct ID";
      SLOG.println(message);
      srvc::publish_log(2, message);
      return;
    }

  } else {

    app::open_flow_calibration_process(id);
    srvc::publish_pressure_report();

    char message[150];
    snprintf(message, 150, "Beginning calibration process. Max dispensation volume: %f liters. Timeout: %d seconds.", app::env.flow_sensor_config.calibration_max_volume, app::env.flow_sensor_config.calibration_timeout);
    SLOG.println(message);
    publish_log(0, message);

  }

}

void srvc::flow_calibration_dispense(byte* payload, unsigned int len) {

  if (!USING_FLOW_SENSOR_) {return;}

  if (!app::env.flag.calibration_flag) {
    char message[150] = "Calibration dispense request denied, calibration not in progress";
    SLOG.println(message);
    srvc::publish_log(2, message);
    return;
  }

  if (app::env.report.calibration_state == 2) {
    char message[150] = "Calibration dispense request denied, calibration dispensation already in progress";
    SLOG.println(message);
    srvc::publish_log(2, message);
    return;

  } else if (app::env.report.calibration_state == 3) {
    char message[150] = "Calibration dispense request denied: last dispensation process ended without measurment";
    SLOG.println(message);
    srvc::publish_log(2, message);
    return;
  }

  StaticJsonDocument<512> json;
  deserialize_json(payload, len, json);

  // Ensure valid input
  if (json["id"].isNull()) {
    char message[150] = "Calibration dispense request denied, no ID was sent";
    SLOG.println(message);
    srvc::publish_log(2, message);
    return;
  } else if (json["id"].as<int>() != app::env.report.calibration_id) {
    char message[150] = "Calibration dispense request denied, incorrect ID";
    SLOG.println(message);
    srvc::publish_log(2, message);
    return;
  }
  if (json["tv"].isNull()) {
    char message[150] = "Calibration dispense request denied, no target volume was sent";
    SLOG.println(message);
    srvc::publish_log(2, message);
    return;
  }
  float target_volume = json["tv"].as<float>();

  if (target_volume > app::env.flow_sensor_config.calibration_max_volume) {
    char message[150] = "Calibration dispense request denied, target volume is above maximum";
    SLOG.println(message);
    srvc::publish_log(2, message);
    return;
  }

  app::begin_calibration_dispense(target_volume);
  srvc::publish_pressure_report();

  char message[150];
  snprintf(message, 150, "Beginning calibration dispensation process with target volume: %f liters", app::env.target.target_calibration_volume);
  SLOG.println(message);
  publish_log(0, message);

}

void srvc::flow_calibration_measure(byte* payload, unsigned int len) {

  if (!USING_FLOW_SENSOR_) {return;}

  if (!app::env.flag.calibration_flag) {
    char message[150] = "Calibration measurement request denied, calibration not in progress";
    SLOG.println(message);
    srvc::publish_log(2, message);
    return;
  }

  if (app::env.report.calibration_state == 1) {
    char message[150] = "Calibration dispense request denied: no pending measurement";
    SLOG.println(message);
    srvc::publish_log(2, message);
    return;
  } else if (app::env.report.calibration_state == 2) {
    char message[150] = "Calibration dispense request denied, calibration dispensation already in progress";
    SLOG.println(message);
    srvc::publish_log(2, message);
    return;
  }

  StaticJsonDocument<512> json;
  deserialize_json(payload, len, json);

  // Ensure valid input
  if (json["id"].isNull()) {
    char message[150] = "Calibration measurement request denied, no ID was sent";
    SLOG.println(message);
    srvc::publish_log(2, message);
    return;
  } else if (json["id"].as<int>() != app::env.report.calibration_id) {
    char message[150] = "Calibration measurement request denied, incorrect ID";
    SLOG.println(message);
    srvc::publish_log(2, message);
    return;
  }
  if (json["mv"].isNull()) {
    char message[150] = "Calibration measurement request denied, no measured volume was sent";
    SLOG.println(message);
    srvc::publish_log(2, message);
    return;
  }

  app::take_calibration_measurement(json["mv"].as<float>());

  char message[150] = "Sucessfully registered calibration measurement";
  SLOG.println(message);
  srvc::publish_log(0, message);

}

void srvc::drain_activate(byte* payload, unsigned int len) {

  if (!USING_DRAIN_VALVE_) {return;}

  // Stop process from being overwritten
  if (app::env.flag.dispense_flag || app::env.flag.drain_flag || app::env.flag.calibration_flag) {
    char message[150] = "Drain request denied, process already in progress. Deactivate with topic: ";
    strncat(message, DEACTIVATE_TOPIC_, 70);
    SLOG.println(message);
    srvc::publish_log(2, message);
    return;
  }

  StaticJsonDocument<512> json;
  deserialize_json(payload, len, json);

  // Ensure valid input
  if ((!json["tt"].isNull() && !json["tv"].isNull()) || (!json["tt"].isNull() && !json["tp"].isNull()) || (!json["tp"].isNull() && !json["tv"].isNull())) {
    char message[150] = "Drain request denied, more than one target was sent";
    SLOG.println(message);
    srvc::publish_log(2, message);
    return;
  }
  if (json["tt"].isNull() && json["tv"].isNull() && json["tp"].isNull()) {
    char message[] = "Drain requested without any target time, pressure, or volume";
    SLOG.println(message);
    srvc::publish_log(2, message);
    return;
  }

  bool activated = false;
  char message[150];
  if (!json["tt"].isNull()) {

    int target_time_seconds = json["tt"].as<int>();
    app::env.target.target_drain_time = target_time_seconds / 1000;
    app::env.target.target_drain_volume = 0;
    app::env.target.target_drain_pressure = 0;
    activated = true;
    snprintf(message, 150, "Beginning drain process with target time: %f", app::env.target.target_drain_time);

  } else if (!json["tv"].isNull()) {

    if (USING_PRESSURE_SENSOR_) {

      app::env.target.target_drain_time = 0;
      app::env.target.target_drain_volume = json["tv"].as<float>();
      app::env.target.target_drain_pressure = 0;
      activated = true;

    } else {
      char message[] = "Unable to set target drain volume as tank pressure sensor not active";
      SLOG.println(message);
      publish_log(2, message);
      return;
    }

  } else if (!json["tp"].isNull()) {

    if (USING_PRESSURE_SENSOR_) {

      app::env.target.target_drain_time = 0;
      app::env.target.target_drain_volume = 0;
      app::env.target.target_drain_pressure = json["tp"].as<float>();
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
      srvc::publish_pressure_report();
      app::open_drain_process();
    }
}

void srvc::publish_drain_summary_report(unsigned long int total_time, float start_pressure, float end_pressure, float start_volume, float end_volume) {

  if (!USING_DRAIN_VALVE_) {return;}

  // Catch failed connection
  if (!app::env.flag.mqtt_connected_flag) {
    SLOG.println("Unable to publish drain summary report. Trying MQTT connection one more time.");
    if (!net::loop_mqtt()) {return;}
  }

  StaticJsonDocument<512> json;

  json["tt"] = (float)total_time / 1000;

  if (USING_PRESSURE_SENSOR_ && (app::env.pressure_sensor_config.report_mode == 1 || app::env.pressure_sensor_config.report_mode == 3)) {
    json["sp"] = start_pressure;
    json["fp"] = end_pressure;
  }
  if (USING_PRESSURE_SENSOR_ && (app::env.pressure_sensor_config.report_mode == 2 || app::env.pressure_sensor_config.report_mode == 3)) {
    json["sv"] = start_volume;
    json["fv"] = end_volume;
  }

  char buffer[256];
  size_t size = serializeJson(json, buffer);
  net::publish(DRAIN_REPORT_SUMMARY_TOPIC_, buffer, size, false);
  srvc::publish_pressure_report();

}

void srvc::publish_pressure_report() {

  if (!USING_PRESSURE_SENSOR_) {return;}

  // Catch failed connection
  if (!app::env.flag.mqtt_connected_flag) {
    SLOG.println("Unable to publish drain summary report. Trying MQTT connection one more time.");
    if (!net::loop_mqtt()) {return;}
  }

  float pressure = pins::read_pressure();
  float volume = app::pressure_to_volume(pressure);


  StaticJsonDocument<512> json;

  if (app::env.pressure_sensor_config.report_mode == 1 || app::env.pressure_sensor_config.report_mode == 3) {
    json["tp"] = pressure;
  }
  if (app::env.pressure_sensor_config.report_mode == 2 || app::env.pressure_sensor_config.report_mode == 3) {
    json["tv"] = volume;
  }

  char buffer[256];
  size_t size = serializeJson(json, buffer);
  net::publish(PRESSURE_REPORT_TOPIC_, buffer, size, false);

}