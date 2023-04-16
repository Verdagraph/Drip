// services.cpp

#include <ArduinoJson.h>

#include "config.h"
#include "pins.h"
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
bool deserialize_json(byte* payload, unsigned int len, StaticJsonDocument<512>& json, const char topic[]) {
  DeserializationError error = deserializeJson(json, payload, len);
  if (error) {
    char message[150];
    snprintf(message, 150, "%s: MQTT payload failed to deserialize with error: %s", topic, error.c_str());
    srvc::warning(message);
    return false;
  }
  return true;
}

void srvc::info(char message[]) {
  SLOG.println(message);
  srvc::publish_log(0, message);
}

void srvc::warning(char message[]) {
  SLOG.println(message);
  srvc::publish_log(1, message);
}

void srvc::error(char message[]) {
  SLOG.println(message);
  srvc::publish_log(2, message);
}

void srvc::dispense_activate(byte* payload, unsigned int len) {

  // Stop process from being overwritten
  if (app::env.flag.dispense_flag || app::env.flag.drain_flag || app::env.flag.calibration_flag) {
    char message[150];
    snprintf(message, 150, "%s: Request denied, process already in progress. Deactivate with topic: %s", DISPENSE_ACTIVATE_TOPIC_, DEACTIVATE_TOPIC_);
    srvc::error(message);
    return;
  }

  // Deserialize
  StaticJsonDocument<512> json;
  if (!deserialize_json(payload, len, json, DISPENSE_ACTIVATE_TOPIC_)) { return; }

  // Validate input
  if (json["tv"].isNull()) {
    char message[150];
    snprintf(message, 150, "%s: Request denied, missing target volume (type: float, name: [\"tv\"])", DISPENSE_ACTIVATE_TOPIC_);
    srvc::error(message);
    return;
  } else if (!json["tv"].is<float>()) {
    char message[150];
    snprintf(message, 150, "%s: Request denied, invalid target volume type (type: float, name: [\"tv\"])", DISPENSE_ACTIVATE_TOPIC_);
    srvc::error(message);
    return;
  }

  float target_output_volume = json["tv"].as<float>();

  char message[150];
  snprintf(message, 150, "%s: Beginning process with target volume: %f liters", DISPENSE_ACTIVATE_TOPIC_, target_output_volume);
  srvc::info(message);

  srvc::publish_pressure_report();
  app::open_dispense_process(target_output_volume);

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
    if (!net::loop_mqtt()) { return; }
  }

  StaticJsonDocument<512> json;
  json["tt"] = (float)total_time / 1000;
  json["vt"] = total_volume;

  if (USING_TANK_) {
    json["tv"] = tank_volume;
    if (USING_SOURCE_) {
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
  srvc::info(message);
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
      net::publish(LOG_TOPIC_, buffer, size, false);
    case 1:
      net::publish(WARNING_TOPIC_, buffer, size, false);
    case 2:
      net::publish(ERROR_TOPIC_, buffer, size, false);
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
  if (!deserialize_json(payload, len, json, CONFIG_CHANGE_TOPIC_)) { return; }

  if (!json["srvc"].isNull()) {
    if (!json["srvc"]["res"].isNull()) {
      if (json["srvc"]["res"].is<float>()) {
        app::env.services_config.data_resolution_l = json["srvc"]["res"].as<int>();
      } else {
        char message[150];
        snprintf(message, 150, "%s: Provided data resolution invalid type (type: float, name: [\"srvc\"][\"res\"])", CONFIG_CHANGE_TOPIC_);
        srvc::error(message);
      }
    }
  }

  if (USING_SOURCE_ && !json["src"].isNull()) {
    if (!json["src"]["rate"].isNull()) {
      if (json["src"]["rate"].is<float>()) {
        app::env.source_config.static_flow_rate = json["src"]["rate"].as<float>();
      } else {
        char message[150];
        snprintf(message, 150, "%s: Provided static flow rate invalid type (type: float, name: [\"src\"][\"rate\"])", CONFIG_CHANGE_TOPIC_);
        srvc::error(message);
      }
    }
  }

  if (USING_TANK_ && !json["tnk"].isNull()) {
    if (!json["tnk"]["time"].isNull()) {
      if (json["tnk"]["time"].is<int>()) {
        app::env.tank_config.tank_timeout = json["tnk"]["time"].as<int>();
      } else {
        char message[150];
        snprintf(message, 150, "%s: Provided tank timeout invalid type (type: int, name: [\"tnk\"][\"time\"])", CONFIG_CHANGE_TOPIC_);
        srvc::error(message);
      }
    }
    if (!json["tnk"]["shape"].isNull()) {
      if (json["tnk"]["shape"].is<int>()) {
        app::env.tank_config.shape_type = json["tnk"]["shape"].as<int>();
      } else {
        char message[150];
        snprintf(message, 150, "%s: Provided tank shape invalid type (type: int, name: [\"tnk\"][\"shape\"])", CONFIG_CHANGE_TOPIC_);
        srvc::error(message);
      }
    }
    if (!json["tnk"]["dim1"].isNull()) {
      if (json["tnk"]["dim1"].is<float>()) {
        app::env.tank_config.dimension_1 = json["tnk"]["dim1"].as<float>();
      } else {
        char message[150];
        snprintf(message, 150, "%s: Provided tank dimension 1 invalid type (type: float, name: [\"tnk\"][\"dim1\"])", CONFIG_CHANGE_TOPIC_);
        srvc::error(message);
      }
    }
    if (!json["tnk"]["dim2"].isNull()) {
      if (json["tnk"]["dim2"].is<float>()) {
        app::env.tank_config.dimension_2 = json["tnk"]["dim2"].as<float>();
      } else {
        char message[150];
        snprintf(message, 150, "%s: Provided tank dimension 2 invalid type (type: float, name: [\"tnk\"][\"dim2\"])", CONFIG_CHANGE_TOPIC_);
        srvc::error(message);
      }
    }
    if (!json["tnk"]["dim3"].isNull()) {
      if (json["tnk"]["dim3"].is<float>()) {
        app::env.tank_config.dimension_3 = json["tnk"]["dim3"].as<float>();
      } else {
        char message[150];
        snprintf(message, 150, "%s: Provided tank dimension 3 invalid type (type: float, name: [\"tnk\"][\"dim3\"])", CONFIG_CHANGE_TOPIC_);
        srvc::error(message);
      }
    }
  }

  if (USING_FLOW_SENSOR_ && !json["flow"].isNull()) {
    if (!json["flow"]["ppl"].isNull()) {
      if (json["flow"]["ppl"].is<float>()) {
        app::env.flow_sensor_config.pulses_per_l = json["flow"]["ppl"].as<float>();
      } else {
        char message[150];
        snprintf(message, 150, "%s: Provided flow sensor pulses per liter invalid type (type: float, name: [\"flow\"][\"ppl\"])", CONFIG_CHANGE_TOPIC_);
        srvc::error(message);
      }
    }
    if (!json["flow"]["max"].isNull()) {
      if (json["flow"]["max"].is<float>()) {
        app::env.flow_sensor_config.max_flow_rate = json["flow"]["max"].as<float>();
      } else {
        char message[150];
        snprintf(message, 150, "%s: Provided flow sensor max flow rate invalid type (type: float, name: [\"flow\"][\"max\"])", CONFIG_CHANGE_TOPIC_);
        srvc::error(message);
      }
    }
    if (!json["flow"]["min"].isNull()) {
      if (json["flow"]["min"].is<float>()) {
        app::env.flow_sensor_config.min_flow_rate = json["flow"]["min"].as<float>();
      } else {
        char message[150];
        snprintf(message, 150, "%s: Provided flow sensor min flow rate invalid type (type: float, name: [\"flow\"][\"min\"])", CONFIG_CHANGE_TOPIC_);
        srvc::error(message);
      }
    }
    if (!json["flow"]["ctime"].isNull()) {
      if (json["flow"]["ctime"].is<float>()) {
        app::env.flow_sensor_config.calibration_timeout = json["flow"]["ctime"].as<float>();
      } else {
        char message[150];
        snprintf(message, 150, "%s: Provided flow sensor calibration timeout invalid type (type: float, name: [\"flow\"][\"ctime\"])", CONFIG_CHANGE_TOPIC_);
        srvc::error(message);
      }
    }
    if (!json["flow"]["cmax"].isNull()) {
      if (json["flow"]["cmax"].is<float>()) {
        app::env.flow_sensor_config.calibration_max_volume = json["flow"]["cmax"].as<float>();
      } else {
        char message[150];
        snprintf(message, 150, "%s: Provided flow sensor max calibration volume invalid type (type: float, name: [\"flow\"][\"cmax\"])", CONFIG_CHANGE_TOPIC_);
        srvc::error(message);
      }
    }
  }

  if (USING_PRESSURE_SENSOR_ && !json["prssr"].isNull()) {
    if (!json["prssr"]["mode"].isNull()) {
      if (json["prssr"]["mode"].is<int>()) {
        app::env.pressure_sensor_config.report_mode = json["prssr"]["mode"].as<int>();
      } else {
        char message[150];
        snprintf(message, 150, "%s: Provided pressure sensor report mode invalid type (type: int, name: [\"flow\"][\"mode\"])", CONFIG_CHANGE_TOPIC_);
        srvc::error(message);
      }
    }
    if (!json["prssr"]["atmo"].isNull()) {
      if (json["prssr"]["atmo"].is<float>()) {
        app::env.pressure_sensor_config.atmosphere_pressure = json["prssr"]["atmo"].as<float>();
      } else {
        char message[150];
        snprintf(message, 150, "%s: Provided pressure sensor atmosphere pressure invalid type (type: int, name: [\"flow\"][\"atmo\"])", CONFIG_CHANGE_TOPIC_);
        srvc::error(message);
      }
    }
  }

  file::save_config(&app::env);
  srvc::publish_config();

  char message[150];
  snprintf(message, 150, "%s: Updated all validated config changes", CONFIG_CHANGE_TOPIC_);
  srvc::info(message);
}

void srvc::settings_reset(byte* payload, unsigned int len) {

  StaticJsonDocument<512> json;
  if (!deserialize_json(payload, len, json, SETTINGS_RESET_TOPIC_)) { return; }

  bool restart = false;

  if (!json["wifi"].isNull()) {

    if (!json["wifi"].is<bool>()) {
      char message[150];
      snprintf(message, 150, "%s: Provided WiFi reset invalid type (type: bool, name: [\"wifi\"])", SETTINGS_RESET_TOPIC_);
      srvc::error(message);
    } else {
      net::reset_wifi_settings();
      restart = true;
    }
  }

  if (!json["mqtt"].isNull()) {

    if (!json["mqtt"].is<bool>()) {
      char message[150];
      snprintf(message, 150, "%s: Provided MQTT reset invalid type (type: bool, name: [\"mqtt\"])", SETTINGS_RESET_TOPIC_);
      srvc::error(message);
    } else {
      net::reset_mqtt_settings();
      restart = true;
    }
  }

  if (restart) { srvc::restart(); }
}

void srvc::flow_calibration_begin(byte* payload, unsigned int len) {

  // Stop process from being overwritten
  if (app::env.flag.dispense_flag || app::env.flag.drain_flag) {
    char message[150];
    snprintf(message, 150, "%s: Request denied, process already in progress. Deactivate with topic: %s", FLOW_SENSOR_CALIBRATE_BEGIN_TOPIC_, DEACTIVATE_TOPIC_);
    srvc::error(message);
    return;
  }

  StaticJsonDocument<512> json;
  if (!deserialize_json(payload, len, json, FLOW_SENSOR_CALIBRATE_BEGIN_TOPIC_)) { return; }

  // Ensure valid input
  if (json["id"].isNull()) {
    char message[150];
    snprintf(message, 150, "%s: Request denied, missing ID (type: int, name: [\"id\"])", FLOW_SENSOR_CALIBRATE_BEGIN_TOPIC_);
    srvc::error(message);
    return;
  } else if (!json["id"].is<int>()) {
    char message[150];
    snprintf(message, 150, "%s: Request denied, invalid ID (type: float, name: [\"id\"])", FLOW_SENSOR_CALIBRATE_BEGIN_TOPIC_);
    srvc::error(message);
    return;
  }
  int id = json["id"].as<int>();

  if (app::env.flag.calibration_flag) {

    if (id == app::env.report.calibration_id) {

      app::close_flow_calibration_process(true);

    } else {
      char message[150];
      snprintf(message, 150, "%s: Request denied, calibration is already in progess. Cancel with a matching ID (type: int, name: [\"id\"])", FLOW_SENSOR_CALIBRATE_BEGIN_TOPIC_);
      srvc::error(message);
      return;
    }

  } else {

    srvc::publish_pressure_report();
    app::open_flow_calibration_process(id);

    char message[150];
    snprintf(message, 150, "%s: Beginning calibration process. Max dispensation volume: %f liters. Timeout: %d seconds.", FLOW_SENSOR_CALIBRATE_BEGIN_TOPIC_, app::env.flow_sensor_config.calibration_max_volume, app::env.flow_sensor_config.calibration_timeout);
    srvc::info(message);
  }
}

void srvc::flow_calibration_dispense(byte* payload, unsigned int len) {

  if (!USING_FLOW_SENSOR_) { return; }

  if (!app::env.flag.calibration_flag) {
    char message[150];
    snprintf(message, 150, "%s: Request denied, calibration not in progress", FLOW_SENSOR_CALIBRATE_DISPENSE_TOPIC_);
    srvc::error(message);
    return;
  }

  if (app::env.report.calibration_state == 2) {
    char message[150];
    snprintf(message, 150, "%s: Request denied, calibration dispensation already in progress", FLOW_SENSOR_CALIBRATE_DISPENSE_TOPIC_);
    srvc::error(message);
    return;

  } else if (app::env.report.calibration_state == 3) {
    char message[150];
    snprintf(message, 150, "%s: Request denied, last calibration dispensation ended without measurment", FLOW_SENSOR_CALIBRATE_DISPENSE_TOPIC_);
    srvc::error(message);
    return;
  }

  StaticJsonDocument<512> json;
  if (!deserialize_json(payload, len, json, FLOW_SENSOR_CALIBRATE_DISPENSE_TOPIC_)) { return; }

  // Ensure valid input

  if (json["id"].isNull()) {
    char message[150];
    snprintf(message, 150, "%s: Request denied, missing ID (type: int, name: [\"id\"])", FLOW_SENSOR_CALIBRATE_DISPENSE_TOPIC_);
    srvc::error(message);
    return;
  } else if (!json["id"].is<int>()) {
    char message[150];
    snprintf(message, 150, "%s: Request denied, invalid ID (type: int, name: [\"id\"])", FLOW_SENSOR_CALIBRATE_DISPENSE_TOPIC_);
    srvc::error(message);
    return;
  } else if (json["id"].as<int>() != app::env.report.calibration_id) {
    char message[150];
    snprintf(message, 150, "%s: Request denied, ID doesn't match (type: int, name: [\"id\"])", FLOW_SENSOR_CALIBRATE_DISPENSE_TOPIC_);
    srvc::error(message);
    return;
  }

  if (json["tv"].isNull()) {
    char message[150];
    snprintf(message, 150, "%s: Request denied, missing target volume (type: float, name: [\"tv\"])", FLOW_SENSOR_CALIBRATE_DISPENSE_TOPIC_);
    srvc::error(message);
    return;
  } else if (!json["tv"].is<float>()) {
    char message[150];
    snprintf(message, 150, "%s: Request denied, invalid target volume (type: float, name: [\"tv\"])", FLOW_SENSOR_CALIBRATE_DISPENSE_TOPIC_);
    srvc::error(message);
    return;
  }
  float target_volume = json["tv"].as<float>();

  if (target_volume > app::env.flow_sensor_config.calibration_max_volume) {
    char message[150];
    ;
    snprintf(message, 150, "%s: Request denied, target volume is above maximum", FLOW_SENSOR_CALIBRATE_DISPENSE_TOPIC_);
    srvc::error(message);
    return;
  }

  char message[150];
  snprintf(message, 150, "%s: Beginning dispensation process with target volume: %f liters", FLOW_SENSOR_CALIBRATE_DISPENSE_TOPIC_, target_volume);
  srvc::info(message);

  srvc::publish_pressure_report();
  app::begin_calibration_dispense(target_volume);

}

void srvc::flow_calibration_measure(byte* payload, unsigned int len) {

  if (!USING_FLOW_SENSOR_) { return; }

  if (!app::env.flag.calibration_flag) {
    char message[150];
    snprintf(message, 150, "%s: Request denied, calibration not in progress", FLOW_SENSOR_CALIBRATE_MEASURE_TOPIC_);
    srvc::error(message);
    return;
  }

  if (app::env.report.calibration_state == 1) {
    char message[150];
    snprintf(message, 150, "%s: Request denied, no pending measurement", FLOW_SENSOR_CALIBRATE_MEASURE_TOPIC_);
    srvc::error(message);
    return;
  } else if (app::env.report.calibration_state == 2) {
    char message[150];
    snprintf(message, 150, "%s: Request denied, dispensation already in progress", FLOW_SENSOR_CALIBRATE_MEASURE_TOPIC_);
    srvc::error(message);
    return;
  }

  StaticJsonDocument<512> json;
  if (!deserialize_json(payload, len, json, FLOW_SENSOR_CALIBRATE_MEASURE_TOPIC_)) { return; }

  // Ensure valid input
  if (json["id"].isNull()) {
    char message[150];
    snprintf(message, 150, "%s: Request denied, missing ID (type: int, name: [\"id\"])", FLOW_SENSOR_CALIBRATE_MEASURE_TOPIC_);
    srvc::error(message);
    return;
  } else if (!json["id"].is<int>()) {
    char message[150];
    snprintf(message, 150, "%s: Request denied, invalid ID (type: int, name: [\"id\"])", FLOW_SENSOR_CALIBRATE_MEASURE_TOPIC_);
    srvc::error(message);
    return;
  } else if (json["id"].as<int>() != app::env.report.calibration_id) {
    char message[150];
    snprintf(message, 150, "%s: Request denied, ID doesn't match (type: int, name: [\"id\"])", FLOW_SENSOR_CALIBRATE_MEASURE_TOPIC_);
    srvc::error(message);
    return;
  }

  if (json["mv"].isNull()) {
    char message[150];
    snprintf(message, 150, "%s: Request denied, missing measured volume (type: float, name: [\"mv\"])", FLOW_SENSOR_CALIBRATE_MEASURE_TOPIC_);
    srvc::error(message);
    return;
  } else if (!json["mv"].is<float>()) {
    char message[150];
    snprintf(message, 150, "%s: Request denied, invalid measured volume (type: float, name: [\"mv\"])", FLOW_SENSOR_CALIBRATE_MEASURE_TOPIC_);
    srvc::error(message);
    return;
  }
  float measured_volume = json["mv"].as<float>();

  app::take_calibration_measurement(measured_volume);

  char message[150];
  snprintf(message, 150, "%s: Registered calibration measurement of: %f liters", FLOW_SENSOR_CALIBRATE_MEASURE_TOPIC_, measured_volume);
  srvc::info(message);
}

void srvc::drain_activate(byte* payload, unsigned int len) {

  if (!USING_DRAIN_VALVE_) { return; }

  // Stop process from being overwritten
  if (app::env.flag.dispense_flag || app::env.flag.drain_flag || app::env.flag.calibration_flag) {
    char message[150];
    snprintf(message, 150, "%s: Request denied, process already in progress. Deactivate with topic: %s", DRAIN_ACTIVATE_TOPIC_, DEACTIVATE_TOPIC_);
    srvc::error(message);
    return;
  }

  StaticJsonDocument<512> json;
  if (!deserialize_json(payload, len, json, DRAIN_ACTIVATE_TOPIC_)) { return; }

  // Ensure valid input
  if ((!json["tt"].isNull() && !json["tv"].isNull()) || (!json["tt"].isNull() && !json["tp"].isNull()) || (!json["tp"].isNull() && !json["tv"].isNull())) {
    char message[150];
    snprintf(message, 150, "%s: Request denied, more than one target was sent", DRAIN_ACTIVATE_TOPIC_);
    srvc::error(message);
    return;
  }
  if (json["tt"].isNull() && json["tv"].isNull() && json["tp"].isNull()) {
    char message[150];
    snprintf(message, 150, "%s: Request denied, no target was sent", DRAIN_ACTIVATE_TOPIC_);
    srvc::error(message);
    return;
  }

  bool activated = false;
  if (!json["tt"].isNull()) {

    if (!json["tt"].is<int>()) {
      char message[150];
      snprintf(message, 150, "%s: Request denied, invalid target time (type: int, name: [\"tt\"])", DRAIN_ACTIVATE_TOPIC_);
      srvc::error(message);
      return;
    }

    int target_time_seconds = json["tt"].as<int>() / 1000;

    srvc::publish_pressure_report();
    app::open_drain_process(target_time_seconds, 0, 0);

    char message[150];
    snprintf(message, 150, "Beginning drain process with target time: %f seconds", target_time_seconds);
    srvc::info(message);

  } else if (!json["tv"].isNull()) {

    if (!USING_PRESSURE_SENSOR_) {
      char message[150];
      snprintf(message, 150, "%s: Request denied, pressure sensor inactive", DRAIN_ACTIVATE_TOPIC_);
      srvc::error(message);
      return;
    }

    if (!json["tv"].is<float>()) {
      char message[150];
      snprintf(message, 150, "%s: Request denied, invalid target volume (type: float, name: [\"tv\"])", DRAIN_ACTIVATE_TOPIC_);
      srvc::error(message);
      return;
    }

    float target_volume = json["tv"].as<float>();

    char message[150];
    snprintf(message, 150, "Beginning drain process with target volume: %f liters", target_volume);
    srvc::info(message);

    srvc::publish_pressure_report();
    app::open_drain_process(0, target_volume, 0);


  } else if (!json["tp"].isNull()) {

    if (!USING_PRESSURE_SENSOR_) {
      char message[150];
      snprintf(message, 150, "%s: Request denied, pressure sensor inactive", DRAIN_ACTIVATE_TOPIC_);
      srvc::error(message);
      return;
    }

    if (!json["tp"].is<float>()) {
      char message[150];
      snprintf(message, 150, "%s: Request denied, invalid target pressure (type: float, name: [\"tp\"])", DRAIN_ACTIVATE_TOPIC_);
      srvc::error(message);
      return;
    }

    float target_pressure = json["tp"].as<float>();

    char message[150];
    snprintf(message, 150, "Beginning drain process with target pressure: %f hectopascals", target_pressure);
    srvc::info(message);

    srvc::publish_pressure_report();
    app::open_drain_process(0, 0, target_pressure);
  }
}

void srvc::publish_drain_summary_report(unsigned long int total_time, float start_pressure, float end_pressure, float start_volume, float end_volume) {

  if (!USING_DRAIN_VALVE_) { return; }

  // Catch failed connection
  if (!app::env.flag.mqtt_connected_flag) {
    SLOG.println("Unable to publish drain summary report. Trying MQTT connection one more time.");
    if (!net::loop_mqtt()) { return; }
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

  if (!USING_PRESSURE_SENSOR_) { return; }

  // Catch failed connection
  if (!app::env.flag.mqtt_connected_flag) {
    SLOG.println("Unable to publish drain summary report. Trying MQTT connection one more time.");
    if (!net::loop_mqtt()) { return; }
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
